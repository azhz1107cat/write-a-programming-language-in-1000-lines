#pragma once
#include "models.hpp"

namespace model {

// Rational.add：有理数加法（self + 传入Rational，返回新Rational）
inline auto rational_add = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_add)");
    assert(args->val.size() == 1 && "function Rational.add need 1 arg");
    
    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_add must be called by Rational object");
    
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    assert(another_rational != nullptr && "Rational.add only supports Rational type argument");
    
    deps::Rational result = self_rational->val + another_rational->val;
    return new Rational(result);
};

// Rational.sub：有理数减法（self - 传入Rational，返回新Rational）
inline auto rational_sub = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_sub)");
    assert(args->val.size() == 1 && "function Rational.sub need 1 arg");
    
    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_sub must be called by Rational object");
    
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    assert(another_rational != nullptr && "Rational.sub only supports Rational type argument");
    
    deps::Rational result = self_rational->val - another_rational->val;
    return new Rational(result);
};

// Rational.mul：有理数乘法（self * 传入Rational，返回新Rational）
inline auto rational_mul = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_mul)");
    assert(args->val.size() == 1 && "function Rational.mul need 1 arg");
    
    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_mul must be called by Rational object");
    
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    assert(another_rational != nullptr && "Rational.mul only supports Rational type argument");
    
    deps::Rational result = self_rational->val * another_rational->val;
    return new Rational(result);
};

// Rational.div：有理数除法（self ÷ 传入Rational，返回新Rational）
inline auto rational_div = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_div)");
    assert(args->val.size() == 1 && "function Rational.div need 1 arg");
    
    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_div must be called by Rational object");
    
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    assert(another_rational != nullptr && "Rational.div only supports Rational type argument");
    
    // 底层deps::Rational已处理除零异常（分子为0抛出invalid_argument）
    deps::Rational result = self_rational->val / another_rational->val;
    return new Rational(result);
};

// Rational.eq：有理数相等判断（self == 传入Rational，返回Bool）
inline auto rational_eq = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_eq)");
    assert(args->val.size() == 1 && "function Rational.eq need 1 arg");
    
    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_eq must be called by Rational object");
    
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    assert(another_rational != nullptr && "Rational.eq only supports Rational type argument");
    
    bool is_equal = self_rational->val == another_rational->val;
    return new Bool(is_equal);
};

// Rational.lt：有理数小于判断（self < 传入Rational，返回Bool）
inline auto rational_lt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_lt)");
    assert(args->val.size() == 1 && "function Rational.lt need 1 arg");
    
    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_lt must be called by Rational object");
    
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    assert(another_rational != nullptr && "Rational.lt only supports Rational type argument");
    
    // 交叉相乘比较
    const auto& a = self_rational->val;
    const auto& b = another_rational->val;
    bool is_less = (a.getNumerator() * b.getDenominator()) < (b.getNumerator() * a.getDenominator());
    return new Bool(is_less);
};

// Rational.gt：有理数大于判断（self > 传入Rational，返回Bool）
inline auto rational_gt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (rational_gt)");
    assert(args->val.size() == 1 && "function Rational.gt need 1 arg");
    
    auto self_rational = dynamic_cast<Rational*>(self);
    assert(self_rational != nullptr && "rational_gt must be called by Rational object");
    
    auto another_rational = dynamic_cast<Rational*>(args->val[0]);
    assert(another_rational != nullptr && "Rational.gt only supports Rational type argument");
    
    // 同lt逻辑，交叉相乘避免精度损失
    const auto& a = self_rational->val;
    const auto& b = another_rational->val;
    bool is_greater = (a.getNumerator() * b.getDenominator()) > (b.getNumerator() * a.getDenominator());
    return new Bool(is_greater);
};

}  // namespace model