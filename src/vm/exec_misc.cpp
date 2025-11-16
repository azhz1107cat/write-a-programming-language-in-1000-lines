
#include "vm.hpp"

namespace kiz {

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
    list_obj->make_ref();  // List 自身引用计std::to_stringstd::to_string((数+1
    op_stack_.push(list_obj);

    DEBUG_OUTPUT("make_list: 打包 " + std::to_string(elem_count) + " 个元素为 List，压栈成功");
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

}
