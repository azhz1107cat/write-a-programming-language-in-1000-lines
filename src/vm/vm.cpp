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
    DEBUG_OUTPUT("call stack length: " + std::to_string(this->call_stack_.size()));
}

void Vm::extend_code(const model::CodeObject* code_object) {
    DEBUG_OUTPUT("exec extend_code...");
    DEBUG_OUTPUT("call stack length: " + std::to_string(this->call_stack_.size()));

    // 合法性校验（避免空指针崩溃）
    assert(code_object != nullptr && "Vm::extend_code: 传入的 code_object 不能为 nullptr");
    assert(!call_stack_.empty() && "Vm::extend_code: 调用栈为空，需先通过 load() 加载模块");

    // 获取全局模块级调用帧（REPL 共享同一个帧，变量持久化）
    auto& curr_frame = *call_stack_.back();
    auto& global_code = *curr_frame.code_object; // 原有全局 CodeObject
    const size_t prev_instr_count = global_code.code.size(); // 记录原有指令总数
    const size_t prev_const_count = global_code.consts.size(); // 原有常量总数（关键：用于索引映射）
    const size_t prev_name_count = global_code.names.size(); // 原有变量名总数

    // 追加指令（原有逻辑不变）
    for (const auto& instr : code_object->code) {
        global_code.code.push_back(instr);
    }
    DEBUG_OUTPUT("extend_code: 追加指令 "
        + std::to_string(code_object->code.size())
        + " 条（累计 "
        + std::to_string(global_code.code.size())
        +" 条）"
    );

    // 修复常量追加：去重+索引映射+引用计数
    size_t actual_const_added = 0; // 实际追加的常量数量（用于日志）
    // 临时映射：局部常量索引 → 全局常量索引（解决索引不匹配问题）
    std::vector<size_t> const_index_map(code_object->consts.size(), 0);

    for (size_t local_idx = 0; local_idx < code_object->consts.size(); ++local_idx) {
        model::Object* new_const = code_object->consts[local_idx];
        if (new_const == nullptr) {
            assert(false && "extend_code: 新常量池中有 nullptr");
            const_index_map[local_idx] = prev_const_count + actual_const_added;
            actual_const_added++;
            continue;
        }

        // 常量去重
        size_t global_idx = prev_const_count + actual_const_added; // 默认新索引
        bool is_duplicate = false;
        for (size_t i = 0; i < global_code.consts.size(); ++i) {
            model::Object* exist_const = global_code.consts[i];
            if (exist_const == nullptr) continue;
            // 精确去重：根据类型+值判断（比 to_string() 更可靠）
            if (typeid(*exist_const) == typeid(*new_const)) {
                if (exist_const->to_string() == new_const->to_string()) {
                    is_duplicate = true;
                    global_idx = i; // 复用已有常量的全局索引
                    break;
                }
            }
        }

        // 非重复常量
        if (!is_duplicate) {
            new_const->make_ref(); // 全局 CodeObject 持有引用，计数+1
            global_code.consts.push_back(new_const);
            this->constant_pool_.push_back(new_const); // VM 常量池同步
            actual_const_added++;
        }

        // 记录映射：局部索引 → 全局索引（后续修正指令中的常量索引）
        const_index_map[local_idx] = global_idx;
    }

    // 修正新指令中的常量索引
    // 遍历新追加的指令，将 LOAD_CONST 的局部索引替换为全局索引
    for (size_t i = prev_instr_count; i < global_code.code.size(); ++i) {
        auto& instr = global_code.code[i];
        if (instr.opc == Opcode::LOAD_CONST && !instr.opn_list.empty()) {
            size_t local_const_idx = instr.opn_list[0];
            // 校验局部索引有效性
            if (local_const_idx >= const_index_map.size()) {
                assert(false && "extend_code: LOAD_CONST 局部索引超出范围");
                continue;
            }
            // 替换为全局索引
            instr.opn_list[0] = const_index_map[local_const_idx];
            DEBUG_OUTPUT("extend_code: 修正 LOAD_CONST 索引：局部 "
                + std::to_string(local_const_idx)
                + " → 全局"
                + std::to_string(instr.opn_list[0]) );
        }
    }

    DEBUG_OUTPUT("extend_code: 追加常量 "
        + std::to_string(actual_const_added) // 显示实际追加数量
        + " 个（累计 "
        + std::to_string(global_code.consts.size())
        +" 个）"
    );

    // 追加变量名
    for (const std::string& new_name : code_object->names) {
        if (std::find(global_code.names.begin(), global_code.names.end(), new_name) == global_code.names.end()) {
            global_code.names.push_back(new_name);
        }
    }
    DEBUG_OUTPUT("extend_code: 追加变量名 "
        + std::to_string(code_object->names.size())
        + " 个（累计 "
        + std::to_string(global_code.names.size())
        +" 个）"
    );

    // 追加行号映射
    for (const auto& lineno : code_object->lineno_map) {
        global_code.lineno_map.push_back(lineno);
    }

    // 执行新追加的指令
    curr_frame.pc = prev_instr_count;
    while (curr_frame.pc < global_code.code.size()) {
        const Instruction& curr_inst = global_code.code[curr_frame.pc];
        this->exec(curr_inst);
        curr_frame.pc++;
    }
    DEBUG_OUTPUT("extend_code: 执行新指令完成（PC 从 "
        + std::to_string(prev_instr_count)
        +" 到 "
        + std::to_string(curr_frame.pc) +"）"
    );

    // 清理临时资源：释放临时 code_object 对常量的引用
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
        default:                      assert(false && "exec: 未知 opcode");
    }
}

} // namespace kiz