namespace model {

// String.add：字符串拼接（self + 传入String，返回新String，不修改原对象）
inline auto str_add = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_add)");
    assert(args->val.size() == 1 && "function String.add need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_add must be called by String object");
    
    auto another_str = dynamic_cast<String*>(args->val[0]);
    assert(another_str != nullptr && "String.add only supports String type argument");
    
    // 拼接并返回新String
    return new String(self_str->val + another_str->val);
};

// String.mul：字符串重复n次（self * n，返回新String，n为非负整数）
inline auto str_mul = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_mul)");
    assert(args->val.size() == 1 && "function String.mul need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_mul must be called by String object");
    
    auto times_int = dynamic_cast<Int*>(args->val[0]);
    assert(times_int != nullptr && "String.mul only supports Int type argument");
    assert(times_int->val >= 0 && "String.mul requires non-negative integer argument");
    
    std::string result;
    int times = times_int->val;
    for (int i = 0; i < times; ++i) {
        result += self_str->val;
    }
    
    return new String(std::move(result));
};

// String.eq：判断两个字符串是否相等 self == x
inline auto str_eq = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_eq)");
    assert(args->val.size() == 1 && "function String.eq need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_eq must be called by String object");
    
    auto another_str = dynamic_cast<String*>(args->val[0]);
    assert(another_str != nullptr && "String.eq only supports String type argument");
    
    return new Bool(self_str->val == another_str->val);
};

// String.contains：判断是否包含子字符串 x in self
inline auto str_contains = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (str_contains)");
    assert(args->val.size() == 1 && "function String.contains need 1 arg");
    
    auto self_str = dynamic_cast<String*>(self);
    assert(self_str != nullptr && "str_contains must be called by String object");
    
    auto sub_str = dynamic_cast<String*>(args->val[0]);
    assert(sub_str != nullptr && "String.contains only supports String type argument");
    
    bool exists = self_str->val.find(sub_str->val) != std::string::npos;
    return new Bool(exists);
};

}  // namespace model