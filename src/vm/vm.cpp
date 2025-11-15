/**
 * @file vm.cpp
 * @brief 虚拟机（VM）核心实现
 * 用于运行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "vm.hpp"

#include "models.hpp"
#include "opcode.hpp"

#include <algorithm>
#include <cassert>

#include "project_debugger.hpp"

namespace kiz {

VmState Vm::load(const model::Module* src_module) {
    // 合法性校验：防止空指针访问
    assert(src_module != nullptr && "Vm::load: 传入的src_module不能为nullptr");
    assert(src_module->code != nullptr && "Vm::load: 模块的CodeObject未初始化（code为nullptr）");

    // 加载指令列表：将模块CodeObject中的指令复制到VM的执行指令池
    this->code_list_ = src_module->code->code;

    // 加载常量池：处理引用计数，避免常量对象被提前释放
    const std::vector<model::Object*>& module_consts = src_module->code->consts;
    for (model::Object* const_obj : module_consts) {
        assert(const_obj != nullptr && "Vm::load: 常量池中有nullptr对象，非法");
        const_obj->make_ref(); // 增加引用计数（VM持有常量的引用）
        this->constant_pool_.push_back(const_obj);
    }

    // 创建模块级调用帧（CallFrame）：模块是顶层执行单元，对应一个顶层调用帧
    auto module_call_frame = std::make_unique<CallFrame>();
    module_call_frame->is_week_scope = false;          // 模块作用域为"强作用域"（非弱作用域）
    module_call_frame->locals = deps::HashMap<model::Object*>(); // 初始空局部变量表
    module_call_frame->pc = 0;                         // 程序计数器初始化为0（从第一条指令开始执行）
    module_call_frame->return_to_pc = this->code_list_.size(); // 执行完所有指令后返回的位置（指令池末尾）
    module_call_frame->name = src_module->name;        // 调用帧名称与模块名一致（便于调试）
    module_call_frame->code_object = src_module->code; // 关联当前模块的CodeObject
    module_call_frame->curr_lineno_map = src_module->code->lineno_map; // 复制行号映射（用于错误定位）
    module_call_frame->names = src_module->code->names; // 复制变量名列表（指令操作数索引对应此列表）

    // 将调用帧压入VM的调用栈
    this->call_stack_.emplace_back(std::move(module_call_frame));

    // 初始化VM执行状态：标记为"就绪"
    this->pc_ = 0;         // 全局PC同步为调用帧初始PC
    this->running_ = true; // 标记VM为运行状态（等待exec触发执行）
    assert(!this->call_stack_.empty() && "Vm::load: 调用栈为空，无法执行指令");
    auto& curr_frame = *this->call_stack_.back(); // 获取当前模块的调用帧（栈顶）
    assert(curr_frame.code_object != nullptr && "Vm::load: 当前调用帧无关联CodeObject");

    // 循环执行当前调用帧下的所有指令（依赖已实现的exec单条指令逻辑）
    const std::vector<Instruction>& frame_instructions = curr_frame.code_object->code;
    while (curr_frame.pc < frame_instructions.size()) {
        // 获取当前要执行的指令（用调用帧的pc索引，确保上下文正确）
        const Instruction& curr_inst = frame_instructions[curr_frame.pc];
        // 调用已实现的exec执行单条指令
        this->exec(curr_inst);
        // 指令执行完成后，更新当前调用帧的pc（指向下一条指令）
        curr_frame.pc++;
    }

    // 模块指令执行完毕，标记VM为非运行状态
    this->running_ = false;

    // 构造并返回当前虚拟机状态
    VmState state;
    // 栈顶：操作数栈非空则为栈顶元素，否则为nullptr
    state.stack_top = op_stack_.empty() ? nullptr : op_stack_.top();
    // 局部变量：当前调用帧的locals，无调用帧则为空
    state.locals = call_stack_.empty()
        ? deps::HashMap<model::Object*>()
        : call_stack_.back()->locals;
    return state;
}

void Vm::exec(const Instruction& instruction) {
    switch (instruction.opc) {
        case Opcode::OP_ADD:          exec_ADD(instruction);          break;
        case Opcode::OP_SUB:          exec_SUB(instruction);          break;
        case Opcode::OP_MUL:          exec_MUL(instruction);          break;
        case Opcode::OP_DIV:          exec_DIV(instruction);          break;
        case Opcode::OP_MOD:          exec_MOD(instruction);          break;
        case Opcode::OP_POW:          exec_POW(instruction);          break;
        case Opcode::OP_NEG:          exec_NEG(instruction);          break;
        case Opcode::OP_EQ:           exec_EQ(instruction);           break;
        case Opcode::OP_GT:           exec_GT(instruction);           break;
        case Opcode::OP_LT:           exec_LT(instruction);           break;
        case Opcode::OP_AND:          exec_AND(instruction);          break;
        case Opcode::OP_NOT:          exec_NOT(instruction);          break;
        case Opcode::OP_OR:           exec_OR(instruction);           break;
        case Opcode::OP_IS:           exec_IS(instruction);           break;
        case Opcode::OP_IN:           exec_IN(instruction);           break;
        case Opcode::MAKE_LIST:       exec_MAKE_LIST(instruction);    break;

        case Opcode::CALL:            exec_CALL(instruction);          break;
        case Opcode::RET:             exec_RET(instruction);           break;
        case Opcode::GET_ATTR:        exec_GET_ATTR(instruction);      break;
        case Opcode::SET_ATTR:        exec_SET_ATTR(instruction);      break;
        case Opcode::LOAD_VAR:        exec_LOAD_VAR(instruction);      break;
        case Opcode::LOAD_CONST:      exec_LOAD_CONST(instruction);    break;
        case Opcode::SET_GLOBAL:      exec_SET_GLOBAL(instruction);    break;
        case Opcode::SET_LOCAL:       exec_SET_LOCAL(instruction);     break;
        case Opcode::SET_NONLOCAL:    exec_SET_NONLOCAL(instruction);  break;
        case Opcode::JUMP:            exec_JUMP(instruction);          break;
        case Opcode::JUMP_IF_FALSE:   exec_JUMP_IF_FALSE(instruction); break;
        case Opcode::THROW:           exec_THROW(instruction);         break;
        case Opcode::POP_TOP:         exec_POP_TOP(instruction);       break;
        case Opcode::SWAP:            exec_SWAP(instruction);          break;
        case Opcode::COPY_TOP:        exec_COPY_TOP(instruction);      break;
        default:                      assert(false && "exec: 未知 opcode");
    }
}

// -------------------------- 算术指令 --------------------------
void Vm::exec_ADD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec add...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_ADD: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_int = dynamic_cast<model::Int*>(a);
    auto* b_int = dynamic_cast<model::Int*>(b);
    if (!a_int || !b_int) {
        assert(false && "OP_ADD: 仅支持Int类型运算");
    }

    auto* result = new model::Int();
    result->val = a_int->val + b_int->val;
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_SUB(const Instruction& instruction) {
    DEBUG_OUTPUT("exec sub...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_SUB: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_int = dynamic_cast<model::Int*>(a);
    auto* b_int = dynamic_cast<model::Int*>(b);
    if (!a_int || !b_int) {
        assert(false && "OP_SUB: 仅支持Int类型运算");
    }

    auto* result = new model::Int();
    result->val = a_int->val - b_int->val;
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_MUL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mul...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_MUL: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_int = dynamic_cast<model::Int*>(a);
    auto* b_int = dynamic_cast<model::Int*>(b);
    if (!a_int || !b_int) {
        assert(false && "OP_MUL: 仅支持Int类型运算");
    }

    auto* result = new model::Int();
    result->val = a_int->val * b_int->val;
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_DIV(const Instruction& instruction) {
    DEBUG_OUTPUT("exec div...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_DIV: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_int = dynamic_cast<model::Int*>(a);
    auto* b_int = dynamic_cast<model::Int*>(b);
    if (!a_int || !b_int) {
        assert(false && "OP_DIV: 仅支持Int类型运算");
    }
    if (b_int->val == deps::BigInt(0)) {
        assert(false && "OP_DIV: 除数不能为0");
    }

    auto* result = new model::Rational(
        static_cast<deps::Rational>(a_int->val.operator/(b_int->val))
    );
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_MOD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec mod...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_MOD: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_int = dynamic_cast<model::Int*>(a);
    auto* b_int = dynamic_cast<model::Int*>(b);
    if (!a_int || !b_int) {
        assert(false && "OP_MOD: 仅支持Int类型运算");
    }
    if (b_int->val == deps::BigInt(0)) {
        assert(false && "OP_MOD: 除数不能为0");
    }

    auto* result = new model::Int();
    result->val = a_int->val % b_int->val;
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_POW(const Instruction& instruction) {
    DEBUG_OUTPUT("exec pow...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_POW: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_int = dynamic_cast<model::Int*>(a);
    auto* b_int = dynamic_cast<model::Int*>(b);
    if (!a_int || !b_int) {
        assert(false && "OP_POW: 仅支持Int类型运算");
    }

    auto* result = new model::Int();
    result->val = a_int->val.pow(b_int->val);
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_NEG(const Instruction& instruction) {
    DEBUG_OUTPUT("exec neg...");
    if (op_stack_.empty()) {
        assert(false && "OP_NEG: 操作数栈为空");
    }
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_int = dynamic_cast<model::Int*>(a);
    if (!a_int) {
        assert(false && "OP_NEG: 仅支持Int类型运算");
    }

    auto* result = new model::Int();
    result->val = a_int->val * deps::BigInt(-1);
    result->make_ref();
    op_stack_.push(result);
}

// -------------------------- 比较指令 --------------------------
void Vm::exec_EQ(const Instruction& instruction) {
    DEBUG_OUTPUT("exec eq...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_EQ: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    bool is_equal = false;
    if (auto* a_int = dynamic_cast<model::Int*>(a); a_int) {
        if (auto* b_int = dynamic_cast<model::Int*>(b); b_int) {
            is_equal = (a_int->val == b_int->val);
        }
    } else if (dynamic_cast<model::Nil*>(a) && dynamic_cast<model::Nil*>(b)) {
        is_equal = true;
    } else if (model::String* a_str = dynamic_cast<model::String*>(a); a_str) {
        if (model::String* b_str = dynamic_cast<model::String*>(b); b_str) {
            is_equal = (a_str->val == b_str->val);
        }
    } else {
        assert(false && "OP_EQ: 不支持的类型比较");
    }

    auto* result = new model::Bool(is_equal);
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_GT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec gt...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_GT: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    bool is_gt = false;
    if (auto* a_int = dynamic_cast<model::Int*>(a); a_int) {
        if (auto* b_int = dynamic_cast<model::Int*>(b); b_int) {
            is_gt = (a_int->val > b_int->val);
        }
    } else {
        assert(false && "OP_GT: 仅支持Int类型比较");
    }

    auto* result = new model::Bool(is_gt);
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_LT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec lt...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_LT: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    bool is_lt = false;
    if (auto* a_int = dynamic_cast<model::Int*>(a); a_int) {
        if (auto* b_int = dynamic_cast<model::Int*>(b); b_int) {
            is_lt = (a_int->val < b_int->val);
        }
    } else {
        assert(false && "OP_LT: 仅支持Int类型比较");
    }

    auto* result = new model::Bool(is_lt);
    result->make_ref();
    op_stack_.push(result);
}

// -------------------------- 逻辑指令 --------------------------
void Vm::exec_AND(const Instruction& instruction) {
    DEBUG_OUTPUT("exec and...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_AND: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_bool = dynamic_cast<model::Bool*>(a);
    auto* b_bool = dynamic_cast<model::Bool*>(b);
    if (!a_bool || !b_bool) {
        assert(false && "OP_AND: 仅支持Bool类型运算");
    }

    auto* result = new model::Bool((a_bool->val && b_bool->val));
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_NOT(const Instruction& instruction) {
    DEBUG_OUTPUT("exec not...");
    if (op_stack_.empty()) {
        assert(false && "OP_NOT: 操作数栈为空");
    }
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_bool = dynamic_cast<model::Bool*>(a);
    if (!a_bool) {
        assert(false && "OP_NOT: 仅支持Bool类型运算");
    }

    auto* result = new model::Bool(!a_bool->val);
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_OR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec or...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_OR: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    auto* a_bool = dynamic_cast<model::Bool*>(a);
    auto* b_bool = dynamic_cast<model::Bool*>(b);
    if (!a_bool || !b_bool) {
        assert(false && "OP_OR: 仅支持Bool类型运算");
    }

    auto* result = new model::Bool((a_bool->val || b_bool->val));
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_IS(const Instruction& instruction) {
    DEBUG_OUTPUT("exec is...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_IS: 操作数栈元素不足（需≥2）");
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();

    bool is_same = (a == b);
    auto* result = new model::Bool(is_same);
    result->make_ref();
    op_stack_.push(result);
}

// -------------------------- 容器指令 --------------------------
void Vm::exec_IN(const Instruction& instruction) {
    DEBUG_OUTPUT("exec in...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_IN: 操作数栈元素不足（需≥2）");
    }
    // ToDo:...
}

// -------------------------- 制作列表 --------------------------
void Vm::exec_MAKE_LIST(const Instruction& instruction) {
    DEBUG_OUTPUT("exec make_list...");

    // 校验：操作数必须包含“要打包的元素个数”
    if (instruction.opn_list.empty()) {
        assert(false && "MAKE_LIST: 无元素个数参数");
    }
    size_t elem_count = instruction.opn_list[0];

    // 校验：栈中元素个数 ≥ 要打包的个数
    if (op_stack_.size() < elem_count) {
        assert(false && ("MAKE_LIST: 栈元素不足（需" + std::to_string(elem_count) +
                        "个，实际" + std::to_string(op_stack_.size()) + "个）").c_str());
    }

    // 步骤1：弹出栈顶 elem_count 个元素（栈是LIFO，弹出顺序是 argN → arg2 → arg1）
    std::vector<model::Object*> elem_list;
    elem_list.reserve(elem_count);  // 预分配空间，避免扩容
    for (size_t i = 0; i < elem_count; ++i) {
        model::Object* elem = op_stack_.top();
        op_stack_.pop();

        // 校验：元素不能为 nullptr
        if (elem == nullptr) {
            assert(false && ("MAKE_LIST: 第" + std::to_string(i) + "个元素为nil（非法）").c_str());
        }

        // 关键：List 要持有元素的引用，所以每个元素 make_ref（引用计数+1）
        elem->make_ref();
        elem_list.push_back(elem);
    }

    // 步骤2：反转元素顺序（恢复原参数顺序：arg1 → arg2 → ... → argN）
    std::reverse(elem_list.begin(), elem_list.end());

    // 步骤3：创建 List 对象，压入栈
    auto* list_obj = new model::List(elem_list);
    list_obj->make_ref();  // List 自身引用计数+1
    op_stack_.push(list_obj);

    DEBUG_OUTPUT("make_list: 打包 " << elem_count << " 个元素为 List，压栈成功");
}

// -------------------------- 函数调用/返回 --------------------------
void Vm::exec_CALL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec call...");

    // 栈中至少需要 2 个元素
    if (op_stack_.size() < 2) {
        assert(false && "CALL: 操作数栈元素不足（需≥2：函数对象 + 参数列表）");
    }
    if (call_stack_.empty()) {
        assert(false && "CALL: 无活跃调用帧");
    }

    // 弹出栈顶元素 : 函数对象
    model::Object* func_obj = op_stack_.top();
    op_stack_.pop();
    func_obj->make_ref();  // 临时持有函数对象，避免中途被释放

    // 弹出栈顶-1元素 : 参数列表
    model::Object* args_obj = op_stack_.top();
    op_stack_.pop();
    auto* args_list = dynamic_cast<model::List*>(args_obj);
    if (!args_list) {
        func_obj->del_ref();  // 释放函数对象引用
        assert(false && "CALL: 栈顶-1元素非List类型（参数必须封装为列表）");
    }

    // 分类型处理函数调用（Function / CppFunction）
    if (const auto* cpp_func = dynamic_cast<model::CppFunction*>(func_obj); cpp_func) {
        // -------------------------- 处理 CppFunction 调用 --------------------------
        DEBUG_OUTPUT("call CppFunction...");

        // 调用 C++ 函数：传入参数列表，获取返回值
        model::Object* return_val = cpp_func->func(args_list);

        // 管理返回值引用计数：返回值压栈前必须 make_ref
        if (return_val != nullptr) {
            return_val->make_ref();
        } else {
            // 若返回空，默认压入 Nil（避免栈异常）
            return_val = new model::Nil();
            return_val->make_ref();
        }

        // 返回值压入操作数栈
        op_stack_.push(return_val);

        // 释放临时引用
        func_obj->del_ref();
        args_obj->del_ref();
    } else if (const auto* func = dynamic_cast<model::Function*>(func_obj); func) {
        // -------------------------- 处理 Function 调用 --------------------------
        DEBUG_OUTPUT("call Function: " + func->name);

        // 校验参数数量
        size_t required_argc = func->argc;
        size_t actual_argc = args_list->val.size();
        if (actual_argc != required_argc) {
            func_obj->del_ref();
            args_obj->del_ref();
            assert(false && ("CALL: 参数数量不匹配（需" + std::to_string(required_argc) +
                            "个，实际" + std::to_string(actual_argc) + "个）").c_str());
        }

        // 创建新调用帧
        auto new_frame = std::make_unique<CallFrame>();
        new_frame->name = func->name;
        new_frame->code_object = func->code;
        new_frame->pc = 0;
        new_frame->return_to_pc = pc_;
        new_frame->names = func->code->names;
        new_frame->is_week_scope = false;

        // 从参数列表中提取参数，存入调用帧 locals
        for (size_t i = 0; i < required_argc; ++i) {
            if (i >= new_frame->names.size()) {
                func_obj->del_ref();
                args_obj->del_ref();
                assert(false && "CALL: 参数名索引超出范围");
            }

            std::string param_name = new_frame->names[i];
            model::Object* param_val = args_list->val[i];  // 从列表取参数

            // 校验参数非空
            if (param_val == nullptr) {
                func_obj->del_ref();
                args_obj->del_ref();
                assert(false && ("CALL: 参数" + std::to_string(i) + "为nil（不允许空参数）").c_str());
            }

            // 增加参数引用计数（存入locals需持有引用）
            param_val->make_ref();
            new_frame->locals.insert(param_name, param_val);
        }

        // 压入新调用帧，更新程序计数器
        call_stack_.emplace_back(std::move(new_frame));
        pc_ = 0;

        // 释放临时引用
        func_obj->del_ref();
        args_obj->del_ref();
    } else {
        // 释放临时引用（类型错误时）
        func_obj->del_ref();
        args_obj->del_ref();
        assert(false && "CALL: 栈顶元素非Function/CppFunction类型");
    }
}

void Vm::exec_RET(const Instruction& instruction) {
    DEBUG_OUTPUT("exec ret...");
    if (call_stack_.size() < 2) {
        assert(false && "RET: 无调用者（顶层调用帧无法返回）");
    }
    std::unique_ptr<CallFrame> curr_frame = std::move(call_stack_.back());
    call_stack_.pop_back();
    CallFrame* caller_frame = call_stack_.back().get();

    model::Object* return_val = new model::Nil();
    return_val->make_ref();
    if (!op_stack_.empty()) {
        return_val->del_ref();
        return_val = op_stack_.top();
        op_stack_.pop();
        return_val->make_ref();
    }

    pc_ = curr_frame->return_to_pc;
    op_stack_.push(return_val);
}

// -------------------------- 属性访问 --------------------------
void Vm::exec_GET_ATTR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec get_attr...");
    if (op_stack_.empty() || instruction.opn_list.empty()) {
        assert(false && "GET_ATTR: 操作数栈为空或无属性名索引");
    }
    model::Object* obj = op_stack_.top();
    op_stack_.pop();
    size_t name_idx = instruction.opn_list[0];
    CallFrame* curr_frame = call_stack_.back().get();

    if (name_idx >= curr_frame->names.size()) {
        assert(false && "GET_ATTR: 属性名索引超出范围");
    }
    std::string attr_name = curr_frame->names[name_idx];

    auto attr_it = obj->attrs.find(attr_name);
    if (attr_it == nullptr) {
        assert(false && "GET_ATTR: 对象无此属性");
    }
    model::Object* attr_val = attr_it->value;
    attr_val->make_ref();
    op_stack_.push(attr_val);
}

void Vm::exec_SET_ATTR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_attr...");
    if (op_stack_.size() < 2 || instruction.opn_list.empty()) {
        assert(false && "SET_ATTR: 操作数栈元素不足或无属性名索引");
    }
    model::Object* attr_val = op_stack_.top();
    op_stack_.pop();
    model::Object* obj = op_stack_.top();
    op_stack_.pop();
    size_t name_idx = instruction.opn_list[0];
    CallFrame* curr_frame = call_stack_.back().get();

    if (name_idx >= curr_frame->names.size()) {
        assert(false && "SET_ATTR: 属性名索引超出范围");
    }
    std::string attr_name = curr_frame->names[name_idx];

    auto attr_it = obj->attrs.find(attr_name);
    if (attr_it != nullptr) {
        attr_it->value->del_ref();
    }
    attr_val->make_ref();
    obj->attrs.insert(attr_name, attr_val);
}

// -------------------------- 变量操作 --------------------------
void Vm::exec_LOAD_VAR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec load_var...");
    if (call_stack_.empty() || instruction.opn_list.empty()) {
        assert(false && "LOAD_VAR: 无调用帧或无变量名索引");
    }
    CallFrame* curr_frame = call_stack_.back().get();
    size_t name_idx = instruction.opn_list[0];
    if (name_idx >= curr_frame->names.size()) {
        assert(false && "LOAD_VAR: 变量名索引超出范围");
    }
    const std::string var_name = curr_frame->names[name_idx];

    const auto var_it = curr_frame->locals.find(var_name);
    if (var_it == nullptr) {
        if (auto builtin_it = builtins.find(var_name)) {
            model::Object* builtin_val = builtin_it->value;
            builtin_val->make_ref();
            op_stack_.push(builtin_val);
            return;
        }
        assert(false && "LOAD_VAR: 局部变量未定义");
    }
    model::Object* var_val = var_it->value;
    var_val->make_ref();
    op_stack_.push(var_val);
}

void Vm::exec_LOAD_CONST(const Instruction& instruction) {
    DEBUG_OUTPUT("exec load_const...");
    if (instruction.opn_list.empty()) {
        assert(false && "LOAD_CONST: 无常量索引");
    }
    size_t const_idx = instruction.opn_list[0];
    if (const_idx >= constant_pool_.size()) {
        assert(false && "LOAD_CONST: 常量索引超出范围");
    }
    model::Object* const_val = constant_pool_[const_idx];
    const_val->make_ref();
    op_stack_.push(const_val);
}

void Vm::exec_SET_GLOBAL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_global...");
    if (call_stack_.empty() || op_stack_.empty() || instruction.opn_list.empty()) {
        assert(false && "SET_GLOBAL: 无调用帧/栈空/无变量名索引");
    }
    CallFrame* global_frame = call_stack_.front().get();
    size_t name_idx = instruction.opn_list[0];
    if (name_idx >= global_frame->names.size()) {
        assert(false && "SET_GLOBAL: 变量名索引超出范围");
    }
    std::string var_name = global_frame->names[name_idx];

    model::Object* var_val = op_stack_.top();
    op_stack_.pop();
    var_val->make_ref();

    auto var_it = global_frame->locals.find(var_name);
    if (var_it != nullptr) {
        var_it->value->del_ref();
    }
    global_frame->locals.insert(var_name, var_val);
}

void Vm::exec_SET_LOCAL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_local...");
    if (call_stack_.empty() || op_stack_.empty() || instruction.opn_list.empty()) {
        assert(false && "SET_LOCAL: 无调用帧/栈空/无变量名索引");
    }
    CallFrame* curr_frame = call_stack_.back().get();
    size_t name_idx = instruction.opn_list[0];
    if (name_idx >= curr_frame->names.size()) {
        assert(false && "SET_LOCAL: 变量名索引超出范围");
    }
    std::string var_name = curr_frame->names[name_idx];

    model::Object* var_val = op_stack_.top();
    op_stack_.pop();
    var_val->make_ref();

    auto var_it = curr_frame->locals.find(var_name);
    if (var_it != nullptr) {
        var_it->value->del_ref();
    }
    curr_frame->locals.insert(var_name, var_val);
}

void Vm::exec_SET_NONLOCAL(const Instruction& instruction) {
    DEBUG_OUTPUT("exec set_nonlocal...");
    if (call_stack_.size() < 2 || op_stack_.empty() || instruction.opn_list.empty()) {
        assert(false && "SET_NONLOCAL: 调用帧不足/栈空/无变量名索引");
    }
    size_t name_idx = instruction.opn_list[0];
    std::string var_name;
    CallFrame* target_frame = nullptr;

    auto frame_it = call_stack_.rbegin();
    ++frame_it;
    for (; frame_it != call_stack_.rend(); ++frame_it) {
        CallFrame* frame = frame_it->get();
        if (name_idx >= frame->names.size()) continue;
        var_name = frame->names[name_idx];
        if (frame->locals.find(var_name)) {
            target_frame = frame;
            break;
        }
    }

    if (!target_frame) {
        assert(false && "SET_NONLOCAL: 未找到非局部变量");
    }

    model::Object* var_val = op_stack_.top();
    op_stack_.pop();
    var_val->make_ref();

    auto var_it = target_frame->locals.find(var_name);
    if (var_it != nullptr) {
        var_it->value->del_ref();
    }
    target_frame->locals.insert(var_name, var_val);
}

// -------------------------- 跳转指令 --------------------------
void Vm::exec_JUMP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec jump...");
    if (instruction.opn_list.empty()) {
        assert(false && "JUMP: 无目标pc索引");
    }
    size_t target_pc = instruction.opn_list[0];
    if (target_pc >= code_list_.size()) {
        assert(false && "JUMP: 目标pc超出字节码范围");
    }
    pc_ = target_pc;
}

void Vm::exec_JUMP_IF_FALSE(const Instruction& instruction) {
    DEBUG_OUTPUT("exec jump_if_false...");
    if (op_stack_.empty() || instruction.opn_list.empty()) {
        assert(false && "JUMP_IF_FALSE: 栈空/无目标pc");
    }
    model::Object* cond = op_stack_.top();
    op_stack_.pop();
    size_t target_pc = instruction.opn_list[0];

    bool need_jump = false;
    if (dynamic_cast<model::Nil*>(cond)) {
        need_jump = true;
    } else if (const auto* cond_bool = dynamic_cast<model::Bool*>(cond); cond_bool) {
        need_jump = !cond_bool->val;
    } else {
        assert(false && "JUMP_IF_FALSE: 条件必须是Nil或Bool");
    }

    if (need_jump) {
        if (target_pc >= code_list_.size()) {
            assert(false && "JUMP_IF_FALSE: 目标pc超出范围");
        }
        pc_ = target_pc;
    }
}

// -------------------------- 异常处理 --------------------------
void Vm::exec_THROW(const Instruction& instruction) {
    DEBUG_OUTPUT("exec throw...");
    // 原逻辑未实现，保留break
}

// -------------------------- 栈操作 --------------------------
void Vm::exec_POP_TOP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec pop_top...");
    if (op_stack_.empty()) {
        assert(false && "POP_TOP: 操作数栈为空");
    }

    model::Object* top = op_stack_.top();
    op_stack_.pop();

    if (top == nullptr) {
        assert(false && "POP_TOP: 栈顶元素为nil（非法）");
        return;
    }

    // 临时打印结果（验证是否正确）
    DEBUG_OUTPUT("pop_top: 弹出结果 = " + top->to_string());

    top->del_ref();
}

void Vm::exec_SWAP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec swap...");
    if (op_stack_.size() < 2) {
        assert(false && "SWAP: 操作数栈元素不足（需≥2）");
    }
    model::Object* a = op_stack_.top();
    op_stack_.pop();
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    op_stack_.push(a);
    op_stack_.push(b);
}

void Vm::exec_COPY_TOP(const Instruction& instruction) {
    DEBUG_OUTPUT("exec copy_top...");
    if (op_stack_.empty()) {
        assert(false && "COPY_TOP: 操作数栈为空");
    }
    model::Object* top = op_stack_.top();
    top->make_ref();
    op_stack_.emplace(top);
}

} // namespace kiz