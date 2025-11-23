#pragma once
#include "models.hpp"

namespace model {

// 整数加法：self + args[0]
inline auto int_add = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments");
    assert(args->val.size() == 1 && "function Int.add need 1 arg");

    const auto self_int = dynamic_cast<Int*>(self);
    assert(self_int!=nullptr && "function Int.add need 1 arg typed Int");
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Int(self_int->val + another_int->val);
    }
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        const auto left_rational = deps::Rational(self_int->val,deps::BigInt(1));
        return new Rational(left_rational + another_rational->val);
    }
    assert(false && "function Int.add second arg need be Rational or Int");
};

// 整数减法：self - args[0]
inline auto int_sub = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_sub)");
    assert(args->val.size() == 1 && "function Int.sub need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Int(self_int->val - another_int->val);
    }
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        const auto left_rational = deps::Rational(self_int->val,deps::BigInt(1));
        return new Rational(left_rational - another_rational->val);
    }
    assert(false && "function Int.sub second arg need be Rational or Int");
};

// 整数乘法：self * args[0]
inline auto int_mul = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_mul)");
    assert(args->val.size() == 1 && "function Int.mul need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Int(self_int->val * another_int->val);
    }
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        const auto left_rational = deps::Rational(self_int->val,deps::BigInt(1));
        return new Rational(left_rational * another_rational->val);
    }
    assert(false && "function Int.mul second arg need be Rational or Int");
};

// 整数除法 self / args[0]
inline auto int_div = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_div)");
    assert(args->val.size() == 1 && "function Int.div need 1 arg");

    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Rational(operator/(self_int->val , another_int->val));
    }
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        const auto left_rational = deps::Rational(self_int->val,deps::BigInt(1));
        return new Rational(left_rational / another_rational->val);
    }
    assert(false && "function Int.div second arg need be Rational or Int");
};

// 整数幂运算：self ^ args[0]（self的args[0]次方）
inline auto int_pow = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_pow)");
    assert(args->val.size() == 1 && "function Int.pow need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto exp_int = dynamic_cast<Int*>(args->val[0]);
    return new Int(self_int->val.pow(exp_int->val));
};

// 整数取模：self % args[0]（余数与除数同号）
inline auto int_mod = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_mod)");
    assert(args->val.size() == 1 && "function Int.mod need 1 arg");
    assert(dynamic_cast<Int*>(args->val[0])->val != deps::BigInt(0) && "mod by zero");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    deps::BigInt remainder = self_int->val % another_int->val;
    // 修正余数符号（确保与除数同号）
    if (remainder != deps::BigInt(0)
        and self_int->val < deps::BigInt(0) != another_int->val < deps::BigInt(0)
    ) {
        remainder += another_int->val;
    }
    return new Int(deps::BigInt(remainder));
};

// 相等判断：self == args[0]（返回Bool对象）
inline auto int_eq = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_eq)");
    assert(args->val.size() == 1 && "function Int.eq need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Bool(self_int->val == another_int->val);
    }
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        const auto left_rational = deps::Rational(self_int->val,deps::BigInt(1));
        return new Bool(left_rational == another_rational->val);
    }
    assert(false && "function Int.eq second arg need be Rational or Int");
};

// 小于判断：self < args[0]（返回Bool对象）
inline auto int_lt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_lt)");
    assert(args->val.size() == 1 && "function Int.lt need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Bool(self_int->val < another_int->val);
    }
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        const auto left_rational = deps::Rational(self_int->val,deps::BigInt(1));
        return new Bool(left_rational < another_rational->val);
    }
    assert(false && "function Int.lt second arg need be Rational or Int");
};

// 大于判断：self > args[0]（返回Bool对象）
inline auto int_gt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_gt)");
    assert(args->val.size() == 1 && "function Int.gt need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Bool(self_int->val > another_int->val);
    }
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        const auto left_rational = deps::Rational(self_int->val,deps::BigInt(1));
        return new Bool(left_rational > another_rational->val);
    }
    assert(false && "function Int.gt second arg need be Rational or Int");
};

}  // namespace model