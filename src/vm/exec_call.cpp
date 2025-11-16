
#include <cassert>

#include "vm.hpp"

namespace kiz {

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
}
