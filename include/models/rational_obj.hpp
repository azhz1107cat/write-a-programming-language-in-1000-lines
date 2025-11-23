#pragma once
#include "models.hpp"

namespace model {

// Rational.add：有理数加法（self + 传入值，支持Rational/Int，返回新Rational）
inline auto rational_add = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_add)");
    assert(args->val.size() == 1 && "function Rational.add need 1 arg");

    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_add must be called by Rational object");

    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        return new Rational(self_rational->val + another_rational->val);
    }

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        deps::Rational rhs_rational(another_int->val, deps::BigInt(1));
        return new Rational(self_rational->val + rhs_rational);
    }

    assert(false && "function Rational.add second arg need be Rational or Int");
};

// Rational.sub：有理数减法（self - 传入值，支持Rational/Int，返回新Rational）
inline auto rational_sub = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_sub)");
    assert(args->val.size() == 1 && "function Rational.sub need 1 arg");

    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_sub must be called by Rational object");

    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        return new Rational(self_rational->val - another_rational->val);
    }

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        deps::Rational rhs_rational(another_int->val, deps::BigInt(1));
        return new Rational(self_rational->val - rhs_rational);
    }

    assert(false && "function Rational.sub second arg need be Rational or Int");
};

// Rational.mul：有理数乘法（self * 传入值，支持Rational/Int，返回新Rational）
inline auto rational_mul = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_mul)");
    assert(args->val.size() == 1 && "function Rational.mul need 1 arg");

    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_mul must be called by Rational object");

    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        return new Rational(self_rational->val * another_rational->val);
    }

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        deps::Rational rhs_rational(another_int->val, deps::BigInt(1));
        return new Rational(self_rational->val * rhs_rational);
    }

    assert(false && "function Rational.mul second arg need be Rational or Int");
};

// Rational.div：有理数除法（self ÷ 传入值，支持Rational/Int，返回新Rational）
inline auto rational_div = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_div)");
    assert(args->val.size() == 1 && "function Rational.div need 1 arg");

    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_div must be called by Rational object");

    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        return new Rational(self_rational->val / another_rational->val);
    }

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        if (another_int->val == deps::BigInt(0)) {
            throw std::invalid_argument("Rational division by zero");
        }
        deps::Rational rhs_rational(another_int->val, deps::BigInt(1));
        return new Rational(self_rational->val / rhs_rational);
    }

    assert(false && "function Rational.div second arg need be Rational or Int");
};

// Rational.eq：有理数相等判断（self == 传入值，支持Rational/Int，返回Bool）
inline auto rational_eq = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_eq)");
    assert(args->val.size() == 1 && "function Rational.eq need 1 arg");

    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_eq must be called by Rational object");

    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        return new Bool(self_rational->val == another_rational->val);
    }

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        deps::Rational rhs_rational(another_int->val, deps::BigInt(1));
        return new Bool(self_rational->val == rhs_rational);
    }

    assert(false && "function Rational.eq second arg need be Rational or Int");
};

// Rational.lt：有理数小于判断（self < 传入值，支持Rational/Int，返回Bool）
inline auto rational_lt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_lt)");
    assert(args->val.size() == 1 && "function Rational.lt need 1 arg");

    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_lt must be called by Rational object");

    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        return new Bool(self_rational->val < another_rational->val);
    }

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        deps::Rational rhs_rational(another_int->val, deps::BigInt(1));
        return new Bool(self_rational->val < rhs_rational);
    }

    assert(false && "function Rational.lt second arg need be Rational or Int");
};

// Rational.gt：有理数大于判断（self > 传入值，支持Rational/Int，返回Bool）
inline auto rational_gt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_gt)");
    assert(args->val.size() == 1 && "function Rational.gt need 1 arg");

    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_gt must be called by Rational object");

    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    if (another_rational) {
        return new Bool(self_rational->val > another_rational->val);
    }

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        deps::Rational rhs_rational(another_int->val, deps::BigInt(1));
        return new Bool(self_rational->val > rhs_rational);
    }

    assert(false && "function Rational.gt second arg need be Rational or Int");
};

}  // namespace model