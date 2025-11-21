namespace model {

// 整数加法：self + args[0]
inline auto int_add = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments");
    assert(args->val.size() == 1 && "function Int.add need 1 arg");
    
    auto another_int_obj = dynamic_cast<Int*>(args->val[0]);
    return new Int (
        dynamic_cast<Int*>(self)->val
        + another_int_obj->val
    );
};

// 整数减法：self - args[0]
inline auto int_sub = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_sub)");
    assert(args->val.size() == 1 && "function Int.sub need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    return new Int(self_int->val - another_int->val);
};

// 整数乘法：self * args[0]
inline auto int_mul = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_mul)");
    assert(args->val.size() == 1 && "function Int.mul need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    return new Int(self_int->val * another_int->val);
};

// 整数除法 self / args[0]
inline auto int_div = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_div)");
    assert(args->val.size() == 1 && "function Int.div need 1 arg");
    assert(dynamic_cast<Int*>(args->val[0])->val != 0 && "division by zero");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    return new Rational(self_int->val , another_int->val);
};

// 整数幂运算：self ^ args[0]（self的args[0]次方）
inline auto int_pow = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_pow)");
    assert(args->val.size() == 1 && "function Int.pow need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto exp_int = dynamic_cast<Int*>(args->val[0]);
    return new Int(self_int.pow(exp_int));
};

// 整数取模：self % args[0]（余数与除数同号）
inline auto int_mod = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_mod)");
    assert(args->val.size() == 1 && "function Int.mod need 1 arg");
    assert(dynamic_cast<Int*>(args->val[0])->val != 0 && "mod by zero");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    int remainder = self_int->val % another_int->val;
    // 修正余数符号（确保与除数同号）
    if (remainder != 0 && (self_int->val < 0) != (another_int->val < 0)) {
        remainder += another_int->val;
    }
    return new Int(remainder);
};

// 相等判断：self == args[0]（返回Bool对象）
inline auto int_eq = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_eq)");
    assert(args->val.size() == 1 && "function Int.eq need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    return new Bool(self_int->val == another_int->val);
};

// 小于判断：self < args[0]（返回Bool对象）
inline auto int_lt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_lt)");
    assert(args->val.size() == 1 && "function Int.lt need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    return new Bool(self_int->val < another_int->val);
};

// 大于判断：self > args[0]（返回Bool对象）
inline auto int_gt = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_gt)");
    assert(args->val.size() == 1 && "function Int.gt need 1 arg");
    
    auto self_int = dynamic_cast<Int*>(self);
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    return new Bool(self_int->val > another_int->val);
};

}  // namespace model