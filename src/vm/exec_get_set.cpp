#include <cassert>

#include "vm.hpp"

namespace kiz {

model::Object* Vm::get_attr(const model::Object* obj, const std::string& attr_name) {
    if (obj == nullptr) assert(false && ("GET_ATTR: 对象无此属性: "+attr_name).c_str());
    const auto attr_it = obj->attrs.find(attr_name);
    auto parent_it = obj->attrs.find("__parent__");
    if (attr_it != nullptr) return attr_it->value;

    if (parent_it != nullptr) return get_attr(parent_it->value, attr_name);

    if (attr_name == "add")      return GET_MAGIC_METHOD(obj, add);
    if (attr_name == "sub") return GET_MAGIC_METHOD(obj, sub);
    if (attr_name == "mul") return GET_MAGIC_METHOD(obj, mul);
    if (attr_name == "div") return GET_MAGIC_METHOD(obj, div);
    if (attr_name == "mod") return GET_MAGIC_METHOD(obj, mod);
    if (attr_name == "pow") return GET_MAGIC_METHOD(obj, pow);
    if (attr_name == "in")  return GET_MAGIC_METHOD(obj, in);
    if (attr_name == "eq")  return GET_MAGIC_METHOD(obj, eq);
    if (attr_name == "lt")  return GET_MAGIC_METHOD(obj, lt);
    if (attr_name == "gt")  return GET_MAGIC_METHOD(obj, gt);

    assert(false && ("GET_ATTR: 对象无此属性: "+attr_name).c_str());
}

// -------------------------- 变量操作 --------------------------
void Vm::exec_LOAD_VAR(const Instruction& instruction) {
    DEBUG_OUTPUT("exec load_var...");
    if (call_stack_.empty() || instruction.opn_list.empty()) {
        assert(false && "LOAD_VAR: 无调用帧或无变量名索引");
    }
    const CallFrame* curr_frame = call_stack_.back().get();
    size_t name_idx = instruction.opn_list[0];
    DEBUG_OUTPUT("current name list len is "+std::to_string(curr_frame->names.size()));

    if (name_idx >= curr_frame->names.size()) {
        assert(false && "LOAD_VAR: 变量名索引超出范围");
    }
    DEBUG_OUTPUT("current local at [" + std::to_string(call_stack_.size()) + "] " +  curr_frame->locals.to_string());
    const std::string var_name = curr_frame->names[name_idx];
    DEBUG_OUTPUT("ok to get var name: " + var_name);

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
    DEBUG_OUTPUT("ok to exec load_var...");
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
    DEBUG_OUTPUT("ok to get load const [" + std::to_string(const_idx) + "]: "+ const_val->to_string());
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
    const std::string var_name = curr_frame->names[name_idx];
    DEBUG_OUTPUT("ok to get var name: " + var_name);

    model::Object* var_val = op_stack_.top();
    op_stack_.pop();
    var_val->make_ref();
    DEBUG_OUTPUT("var val: " + var_val->to_string());

    auto var_it = curr_frame->locals.find(var_name);
    if (var_it != nullptr) {
        var_it->value->del_ref();
    }
    curr_frame->locals.insert(var_name, var_val);
    DEBUG_OUTPUT("ok to set_local...");
    DEBUG_OUTPUT("current local at [" + std::to_string(call_stack_.size()) + "] " +  curr_frame->locals.to_string());
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

    model::Object* attr_val = get_attr(obj, attr_name);
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

}
