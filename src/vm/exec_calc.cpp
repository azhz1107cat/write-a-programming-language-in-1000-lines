#include <tuple>

#include "models.hpp"
#include "kiz.hpp"
#include "vm.hpp"

namespace kiz {

std::tuple<model::Object*, model::Object*> Vm::fetch_two_from_stack_top(
    const std::string& curr_instruction_name
) {
    DEBUG_OUTPUT("exec " + curr_instruction_name + " then fetch two from stack top");
    if (op_stack_.size() < 2) {
        assert(false && (curr_instruction_name + ": 操作数栈元素不足（需≥2）").data());
    }
    model::Object* b = op_stack_.top();
    op_stack_.pop();
    model::Object* a = op_stack_.top();
    op_stack_.pop();
    return {a, b};
}

// -------------------------- 算术指令 --------------------------
void Vm::exec_ADD(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec add...");
    auto [a, b] = fetch_two_from_stack_top("add");
    DEBUG_OUTPUT("a is " + a->to_string() + ", b is " + b->to_string());

    call_function(get_attr(a, "__add__"), new model::List({b}), a);
    DEBUG_OUTPUT("success to call function");


    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_SUB(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec sub...");
    auto [a, b] = fetch_two_from_stack_top("sub");

    call_function(get_attr(a, "__sub__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_MUL(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec mul...");
    auto [a, b] = fetch_two_from_stack_top("mul");

    call_function(get_attr(a, "__mul__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_DIV(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec div...");
    auto [a, b] = fetch_two_from_stack_top("div");

    call_function(get_attr(a, "__div__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_MOD(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec mod...");
    auto [a, b] = fetch_two_from_stack_top("mod");

    call_function(get_attr(a, "__mod__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_POW(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec pow...");
    auto [a, b] = fetch_two_from_stack_top("pow");

    call_function(get_attr(a, "__pow__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_NEG(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec neg...");
    auto [a, b] = fetch_two_from_stack_top("neg");
    // check_has_magic(a, "neg");
    //
    // call_function(a->dyn_cast()->magic_add, new model::List({b}), a);
    // model::Object* result = op_stack_.top();
    //
    // if (raw_call_stack_count != call_stack_.size()) {
    //     exec_RET({});
    // }
    // result->make_ref();
    // op_stack_.push(result);
}

// -------------------------- 比较指令 --------------------------
void Vm::exec_EQ(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec eq...");
    auto [a, b] = fetch_two_from_stack_top("eq");

    call_function(get_attr(a, "__eq__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_GT(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec gt...");
    auto [a, b] = fetch_two_from_stack_top("gt");

    call_function(get_attr(a, "__gt__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

void Vm::exec_LT(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();

    DEBUG_OUTPUT("exec lt...");
    auto [a, b] = fetch_two_from_stack_top("lt");

    call_function(get_attr(a, "__lt__"), new model::List({b}), a);

    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

// -------------------------- 逻辑指令 --------------------------
void Vm::exec_AND(const Instruction& instruction) {
    // a: ...
}

void Vm::exec_NOT(const Instruction& instruction) {
    // a: ...
}

void Vm::exec_OR(const Instruction& instruction) {
    // a: ...
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
}

// -------------------------- 容器指令 --------------------------
void Vm::exec_IN(const Instruction& instruction) {
    const auto raw_call_stack_count = call_stack_.size();
    DEBUG_OUTPUT("exec in...");
    auto [a, b] = fetch_two_from_stack_top("in");

    call_function(get_attr(a, "__contains__"), new model::List({b}), a);


    if (raw_call_stack_count != call_stack_.size()) {
        exec_RET({});
    }
}

}
