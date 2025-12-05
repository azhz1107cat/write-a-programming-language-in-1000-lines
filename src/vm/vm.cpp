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

#include "kiz.hpp"

namespace kiz {

deps::HashMap<model::Object*> Vm::builtins{};

Vm::Vm(const std::string& file_path) : file_path(file_path) {
    DEBUG_OUTPUT("registering builtin functions...");
#define KIZ_FUNC(n) builtins.insert(#n, new model::CppFunction(builtin_objects::n))
    KIZ_FUNC(print);
    KIZ_FUNC(input);
    KIZ_FUNC(isinstance);
#undef KIZ_FUNC

    DEBUG_OUTPUT("registering builtin objects...");
    builtins.insert("obj", model::based_obj);

    model::based_bool->attrs.insert("__parent__", model::based_obj);
    model::based_int->attrs.insert("__parent__", model::based_obj);
    model::based_nil->attrs.insert("__parent__", model::based_obj);
    model::based_rational->attrs.insert("__parent__", model::based_obj);
    model::based_function->attrs.insert("__parent__", model::based_obj);
    model::based_dict->attrs.insert("__parent__", model::based_obj);
    model::based_list->attrs.insert("__parent__", model::based_obj);
    model::based_str->attrs.insert("__parent__", model::based_obj);

    DEBUG_OUTPUT("registering magic methods...");
    // Object 基类 __eq__
    based_obj->attrs.insert("__eq__", new CppFunction([](const Object* self, const List* args) -> Object* {
        const auto other_obj = get_one_arg(args);
        return new Bool(self == other_obj);
    }));

    // Bool 类型魔法方法
    based_bool->attrs.insert("__eq__", new CppFunction(bool_eq));

    // Nil 类型魔法方法
    based_nil->attrs.insert("__eq__", new CppFunction(nil_eq));

    // Int 类型魔法方法
    based_int->attrs.insert("__add__", new CppFunction(int_add));
    based_int->attrs.insert("__sub__", new CppFunction(int_sub));
    based_int->attrs.insert("__mul__", new CppFunction(int_mul));
    based_int->attrs.insert("__div__", new CppFunction(int_div));
    based_int->attrs.insert("__mod__", new CppFunction(int_mod));
    based_int->attrs.insert("__pow__", new CppFunction(int_pow));
    based_int->attrs.insert("__gt__", new CppFunction(int_gt));
    based_int->attrs.insert("__lt__", new CppFunction(int_lt));
    based_int->attrs.insert("__eq__", new CppFunction(int_eq));

    // Rational 类型魔法方法
    based_rational->attrs.insert("__add__", new CppFunction(rational_add));
    based_rational->attrs.insert("__sub__", new CppFunction(rational_sub));
    based_rational->attrs.insert("__mul__", new CppFunction(rational_mul));
    based_rational->attrs.insert("__div__", new CppFunction(rational_div));
    based_rational->attrs.insert("__gt__", new CppFunction(rational_gt));
    based_rational->attrs.insert("__lt__", new CppFunction(rational_lt));
    based_rational->attrs.insert("__eq__", new CppFunction(rational_eq));

    // Dictionary 类型魔法方法
    based_dict->attrs.insert("__add__", new CppFunction(dict_add));
    based_dict->attrs.insert("__contains__", new CppFunction(dict_contains));

    // List 类型魔法方法
    based_list->attrs.insert("__add__", new CppFunction(list_add));
    based_list->attrs.insert("__mul__", new CppFunction(list_mul));
    based_list->attrs.insert("__contains__", new CppFunction(list_contains));
    based_list->attrs.insert("__eq__", new CppFunction(list_eq));

    // String 类型魔法方法
    based_str->attrs.insert("__add__", new CppFunction(str_add));
    based_str->attrs.insert("__mul__", new CppFunction(str_mul));
    based_str->attrs.insert("__contains__", new CppFunction(str_contains));
    based_str->attrs.insert("__eq__", new CppFunction(str_eq));

    builtins.insert("int", model::based_int);
    builtins.insert("bool", model::based_bool);
    builtins.insert("rational", model::based_rational);
    builtins.insert("list", model::based_list);
    builtins.insert("dict", model::based_dict);
    builtins.insert("str", model::based_str);
    builtins.insert("function", model::based_function);
    builtins.insert("nil", model::based_nil);
}

void Vm::load(model::Module* src_module) {
    DEBUG_OUTPUT("loading module...");
    // 合法性校验：防止空指针访问
    assert(src_module != nullptr && "Vm::run_module: 传入的src_module不能为nullptr");
    assert(src_module->code != nullptr && "Vm::run_module: 模块的CodeObject未初始化（code为nullptr）");
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
    this->running_ = true; // 标记VM为运行状态（等待exec触发执行）
    assert(!this->call_stack_.empty() && "Vm::load: 调用栈为空，无法执行指令");
    auto& module_frame = *this->call_stack_.back(); // 获取当前模块的调用帧（栈顶）
    assert(module_frame.code_object != nullptr && "Vm::load: 当前调用帧无关联CodeObject");

    // 循环执行当前调用帧下的所有指令
    while (!call_stack_.empty() && running_) {
        auto& curr_frame = *call_stack_.back();
        // 检查当前帧是否执行完毕
        if (curr_frame.pc >= curr_frame.code_object->code.size()) {
            // 非模块帧则弹出，模块帧则退出循环
            if (call_stack_.size() > 1) {
                call_stack_.pop_back();
            } else {
                break;
            }
            continue;
        }

        // 执行当前指令
        const Instruction& curr_inst = curr_frame.code_object->code[curr_frame.pc];
        this->exec(curr_inst);
        DEBUG_OUTPUT("curr inst is "+opcode_to_string(curr_inst.opc));

        // 调试输出栈顶
        DEBUG_OUTPUT("current stack top : " +
            (op_stack_.empty() ? " " : op_stack_.top()->to_string())
        );

        // PC自增
        if (!(curr_inst.opc == Opcode::JUMP || curr_inst.opc == Opcode::JUMP_IF_FALSE || curr_inst.opc == Opcode::RET)) {
            curr_frame.pc++;
        }
    }

    DEBUG_OUTPUT("call stack length: " + std::to_string(this->call_stack_.size()));
}

void Vm::extend_code(const model::CodeObject* code_object) {
    DEBUG_OUTPUT("exec extend_code (覆盖模式)...");
    DEBUG_OUTPUT("call stack length: " + std::to_string(this->call_stack_.size()));

    // 合法性校验
    assert(code_object != nullptr && "Vm::extend_code: 传入的 code_object 不能为 nullptr");
    assert(!call_stack_.empty() && "Vm::extend_code: 调用栈为空，需先通过 load() 加载模块");

    // 获取全局模块级调用帧（REPL 共享同一个帧）
    auto& curr_frame = *call_stack_.back();
    auto& global_code_obj = *curr_frame.code_object; // 原有全局 CodeObject
    const size_t prev_instr_count = global_code_obj.code.size(); // 记录原有指令总数（用于后续执行新指令）

    // ========== 覆盖：常量池 ==========
    // 清理原有全局常量池
    for (model::Object* old_const : global_code_obj.consts) {
        if (old_const != nullptr) {
            old_const->del_ref();
        }
    }
    // 清空 VM 同步常量池
    const auto prev_const_count =this->constant_pool_.size();
    this->constant_pool_.clear();
    // 清空全局常量池，用新 code_object 的常量池覆盖（添加引用计数）
    global_code_obj.consts.clear();
    for (model::Object* new_const : code_object->consts) {
        if (new_const == nullptr) {
            assert(false && "extend_code: 新常量池中有 nullptr");
            global_code_obj.consts.push_back(nullptr);
            continue;
        }
        new_const->make_ref(); // 全局 CodeObject 持有该常量，引用计数+1
        global_code_obj.consts.push_back(new_const);
        this->constant_pool_.push_back(new_const); // VM 常量池同步覆盖
    }
    DEBUG_OUTPUT("extend_code: 覆盖常量池：原有 "
        + std::to_string(prev_const_count)
        + " 个 → 新 "
        + std::to_string(global_code_obj.consts.size())
        + " 个"
    );

    // ========== 覆盖：名称表 ==========
    const size_t prev_name_count = global_code_obj.names.size();
    global_code_obj.names.clear();
    global_code_obj.names = code_object->names; // 覆盖 CodeObject 的 names
    // 同步更新 CallFrame 的 names（关键！之前缺失这一步）
    curr_frame.names = global_code_obj.names;
    DEBUG_OUTPUT("extend_code: 覆盖名称表：原有 "
        + std::to_string(prev_name_count)
        + " 个 → 新 "
        + std::to_string(global_code_obj.names.size())
        + " 个（CallFrame names 同步更新）"
    );

    // ========== 覆盖：行号映射 ==========
    global_code_obj.lineno_map.clear();
    global_code_obj.lineno_map = code_object->lineno_map; // 覆盖 CodeObject 的 lineno_map
    // 同步更新 CallFrame 的 curr_lineno_map（关键！避免行号映射错误）
    curr_frame.curr_lineno_map = global_code_obj.lineno_map;
    DEBUG_OUTPUT("extend_code: 覆盖行号映射：新 "
        + std::to_string(global_code_obj.lineno_map.size())
        + " 条（CallFrame lineno_map 同步更新）"
    );

    // ========== 追加指令 ==========
    const size_t new_instr_count = code_object->code.size();
    for (const auto& instr : code_object->code) {
        global_code_obj.code.push_back(instr);
    }
    DEBUG_OUTPUT("extend_code: 追加指令 "
        + std::to_string(new_instr_count)
        + " 条（累计 "
        + std::to_string(global_code_obj.code.size())
        + " 条）"
    );

    // ========== 执行新追加的指令 ==========
    curr_frame.pc = prev_instr_count; // 从原有指令末尾开始执行新指令
    while (curr_frame.pc < global_code_obj.code.size()) {
        const Instruction& curr_inst = global_code_obj.code[curr_frame.pc];
        this->exec(curr_inst);
        curr_frame.pc++;
    }
    DEBUG_OUTPUT("extend_code: 执行新指令完成（PC 从 "
        + std::to_string(prev_instr_count)
        + " 到 "
        + std::to_string(curr_frame.pc)
        + "）"
    );

    // ========== 清理临时资源：释放新 code_object 对常量的引用 ==========
    for (model::Object* new_const : code_object->consts) {
        if (new_const != nullptr) {
            new_const->del_ref();
        }
    }
}

VmState Vm::get_vm_state() {
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
        case Opcode::STOP:            exec_STOP(instruction);           break;
        default:                      assert(false && "exec: 未知 opcode");
    }
}

} // namespace kiz