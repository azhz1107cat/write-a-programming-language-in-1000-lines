#include "models.hpp"
#include "kiz.hpp"
#include "vm.hpp"

namespace kiz {

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
void Vm::exec_IN(const Instruction& instruction) const {
    DEBUG_OUTPUT("exec in...");
    if (op_stack_.size() < 2) {
        assert(false && "OP_IN: 操作数栈元素不足（需≥2）");
    }
    // ToDo:...
}

}
