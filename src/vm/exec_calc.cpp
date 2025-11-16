#include <tuple>

#include "models.hpp"
#include "kiz.hpp"
#include "vm.hpp"

namespace kiz {

std::tuple<model::Object*, model::Object*> Vm::fetch_two_from_stack_top(
    const std::string& curr_instruction_name
) {
    DEBUG_OUTPUT("exec " + curr_instruction_name + "...");
    if (op_stack_.size() < 2) {
        assert(false && (curr_instruction_name + ": 操作数栈元素不足（需≥2）").data());
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();
    return {a, b};
}

bool Vm::check_has_magic(model::Object* a, const std::string& magic_method_name) {
    if (a == nullptr) return false;

    // 魔法方法名 : 成员变量的检查函数
    static const std::unordered_map<std::string, std::function<bool(model::Object*)>> magic_checkers = {
        {"add", [](auto* obj) { return obj->magic_add != nullptr; }},
        {"sub", [](auto* obj) { return obj->magic_sub != nullptr; }},
        {"mul", [](auto* obj) { return obj->magic_mul != nullptr; }},
        {"div", [](auto* obj) { return obj->magic_div != nullptr; }},
        {"pow", [](auto* obj) { return obj->magic_pow != nullptr; }},
        {"mod", [](auto* obj) { return obj->magic_mod != nullptr; }},
        {"in",  [](auto* obj) { return obj->magic_in != nullptr; }},
        {"bool",[](auto* obj) { return obj->magic_bool != nullptr; }},
        {"eq",  [](auto* obj) { return obj->magic_eq != nullptr; }},
        {"lt",  [](auto* obj) { return obj->magic_lt != nullptr; }},
        {"gt",  [](auto* obj) { return obj->magic_gt != nullptr; }}
    };

    const auto it = magic_checkers.find(magic_method_name);
    if (it == magic_checkers.end()) {
        assert(false && ("check_has_magic: 未知的魔法方法名：" + magic_method_name).data());
    }

    return it->second(a);
}

// -------------------------- 算术指令 --------------------------
void Vm::exec_ADD(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec and...");
    auto [a, b] = fetch_two_from_stack_top("add");
    check_has_magic(a, "add");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_SUB(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec sub...");
    auto [a, b] = fetch_two_from_stack_top("sub");
    check_has_magic(a, "sub");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_MUL(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec mul...");
    auto [a, b] = fetch_two_from_stack_top("mul");
    check_has_magic(a, "mul");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_DIV(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec div...");
    auto [a, b] = fetch_two_from_stack_top("div");
    check_has_magic(a, "div");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_MOD(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec mod...");
    auto [a, b] = fetch_two_from_stack_top("mod");
    check_has_magic(a, "add");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_POW(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec pow...");
    auto [a, b] = fetch_two_from_stack_top("pow");
    check_has_magic(a, "pow");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_NEG(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec neg...");
    auto [a, b] = fetch_two_from_stack_top("neg");
    check_has_magic(a, "neg");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

// -------------------------- 比较指令 --------------------------
void Vm::exec_EQ(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec eq...");
    auto [a, b] = fetch_two_from_stack_top("eq");
    check_has_magic(a, "eq");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_GT(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec gt...");
    auto [a, b] = fetch_two_from_stack_top("gt");
    check_has_magic(a, "gt");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

void Vm::exec_LT(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec lt...");
    auto [a, b] = fetch_two_from_stack_top("lt");
    check_has_magic(a, "lt");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

// -------------------------- 逻辑指令 --------------------------
void Vm::exec_AND(const Instruction& instruction) {
    // Todo: ...
}

void Vm::exec_NOT(const Instruction& instruction) {
    // Todo: ...
}

void Vm::exec_OR(const Instruction& instruction) {
    // Todo: ...
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
    const auto raw_call_stack_count = call_stack_.size();
    DEBUG_OUTPUT("exec in...");
    auto [a, b] = fetch_two_from_stack_top("in");
    check_has_magic(a, "in");

    call_function(a->magic_add, new model::List({a, b}));
    model::Object* result = op_stack_.top();


    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
    result->make_ref();
    op_stack_.push(result);
}

}
