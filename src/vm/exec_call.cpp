
#include <cassert>

#include "vm.hpp"

namespace kiz {

void Vm::call_function(model::Object* func_obj, model::Object* args_obj, model::Object* self=nullptr){
    auto* args_list = dynamic_cast<model::List*>(args_obj);

    assert(func_obj != nullptr);
    assert(args_obj != nullptr);

    if (!args_list) {
        func_obj->del_ref();  // 释放函数对象引用
        assert(false && "CALL: 栈顶-1元素非List类型（参数必须封装为列表）");
    }
    DEBUG_OUTPUT("start to call function");

    // 分类型处理函数调用（Function / CppFunction）
    if (const auto* cpp_func = dynamic_cast<model::CppFunction*>(func_obj)) {
        // -------------------------- 处理 CppFunction 调用 --------------------------
        DEBUG_OUTPUT("start to call CppFunction");
        DEBUG_OUTPUT("call CppFunction"
            + cpp_func->to_string()
            + "(self=" + (self ? self->to_string() : "nullptr")
            + ", "+ args_obj->to_string() + ")"
            );

        model::Object* return_val = cpp_func->func(self, args_list);

        DEBUG_OUTPUT("success to get the result of CppFunction");

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
        DEBUG_OUTPUT("ok to call CppFunction...");
        DEBUG_OUTPUT("CppFunction return: " + return_val->to_string());
    } else if (const auto* func = dynamic_cast<model::Function*>(func_obj)) {
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

        constant_pool_ = func->code->consts;

        // 创建新调用帧
        auto new_frame = std::make_unique<CallFrame>();
        new_frame->name = func->name;
        new_frame->code_object = func->code;
        new_frame->pc = 0;
        new_frame->return_to_pc = call_stack_.back()->pc + 1;
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

    DEBUG_OUTPUT("弹出函数对象: " + func_obj->to_string());
    DEBUG_OUTPUT("弹出参数列表: " + args_obj->to_string());

    call_function(func_obj, args_obj);

}

void Vm::exec_CALL_METHOD(const Instruction& instruction) {
    DEBUG_OUTPUT("exec call method...");

    // 弹出栈顶元素 : 源对象
    auto obj = op_stack_.top();
    op_stack_.pop();
    obj->make_ref();

    // 弹出栈顶-1元素 : 参数列表
    model::Object* args_obj = op_stack_.top();
    op_stack_.pop();

    DEBUG_OUTPUT("弹出对象: " + obj->to_string());
    DEBUG_OUTPUT("弹出参数列表: " + args_obj->to_string());

    auto func_obj = get_attr(obj, instruction[0]);
    func_obj->make_ref();

    DEBUG_OUTPUT("获取函数对象: " + func_obj->to_string());

    call_function(func_obj, args_obj, obj);

}

void Vm::exec_RET(const Instruction& instruction) {
    DEBUG_OUTPUT("exec ret...");
    // 兼容顶层调用帧返回
    if (call_stack_.size() < 2) {
        if (!op_stack_.empty()) {
            op_stack_.pop(); // 清理栈顶返回值
        }
        call_stack_.pop_back(); // 弹出最后一个帧
        return;
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

    constant_pool_ = caller_frame->code_object->consts;

    caller_frame->pc = curr_frame->return_to_pc;
    op_stack_.push(return_val);
}
}
