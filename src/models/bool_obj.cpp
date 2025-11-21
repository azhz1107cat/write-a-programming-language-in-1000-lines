namespace model {

// 布尔值相等判断：self == args[0]（仅支持Bool与Bool比较）
inline auto bool_eq = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (bool_eq)");
    assert(args->val.size() == 1 && "function Bool.eq need 1 arg");
    
    auto self_bool = dynamic_cast<Bool*>(self);
    auto another_bool = dynamic_cast<Bool*>(args->val[0]);
    assert(another_bool != nullptr && "Bool.eq only supports Bool type argument");
    
    return new Bool(self_bool->val == another_bool->val);
};

}  // namespace model
