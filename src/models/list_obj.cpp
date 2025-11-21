namespace model {

//  List.add：拼接另一个List（self + 传入List，返回新List）
inline auto list_add = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_add)");
    assert(args->val.size() == 1 && "function List.add need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_add must be called by List object");
    
    auto another_list = dynamic_cast<List*>(args->val[0]);
    assert(another_list != nullptr && "List.add only supports List type argument");
    
    // 浅拷贝
    std::vector<Object*> new_vals = self_list->val;
    new_vals.insert(new_vals.end(), another_list->val.begin(), another_list->val.end());
    
    return new List(std::move(new_vals));
};

//LList.mul：重复自身n次 self * n
inline auto list_mul = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_mul)");
    assert(args->val.size() == 1 && "function List.mul need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_mul must be called by List object");
    
    auto times_int = dynamic_cast<Int*>(args->val[0]);
    assert(times_int != nullptr && "List.mul only supports Int type argument");
    assert(times_int->val >= 0 && "List.mul requires non-negative integer argument");
    
    std::vector<Object*> new_vals;
    int times = times_int->val;
    for (int i = 0; i < times; ++i) {
        new_vals.insert(new_vals.end(), self_list->val.begin(), self_list->val.end());
    }
    
    return new List(std::move(new_vals));
};

// 3. List.eq：判断两个List是否相等
inline auto list_eq = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_eq)");
    assert(args->val.size() == 1 && "function List.eq need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_eq must be called by List object");
    
    auto another_list = dynamic_cast<List*>(args->val[0]);
    assert(another_list != nullptr && "List.eq only supports List type argument");
    
    // 比较元素个数，不同直接返回false
    if (self_list->val.size() != another_list->val.size()) {
        return new Bool(false);
    }
    
    // 逐个比较元素（类型一致 + 值相等才视为相等）
    for (size_t i = 0; i < self_list->val.size(); ++i) {
        Object* self_elem = self_list->val[i];
        Object* another_elem = another_list->val[i];
        
        // 元素类型匹配校验 + 值比较
        if (auto self_int = dynamic_cast<Int*>(self_elem); self_int) {
            auto another_int = dynamic_cast<Int*>(another_elem);
            if (!another_int || self_int->val != another_int->val) return new Bool(false);
        } else if (auto self_bool = dynamic_cast<Bool*>(self_elem); self_bool) {
            auto another_bool = dynamic_cast<Bool*>(another_elem);
            if (!another_bool || self_bool->val != another_bool->val) return new Bool(false);
        } else if (auto self_nil = dynamic_cast<Nil*>(self_elem); self_nil) {
            auto another_nil = dynamic_cast<Nil*>(another_elem);
            if (!another_nil) return new Bool(false);
        } else {
            // 不支持的元素类型，直接视为不相等
            return new Bool(false);
        }
    }
    
    return new Bool(true);
};

}  // namespace model
