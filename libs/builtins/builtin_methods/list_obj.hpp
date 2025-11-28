#pragma once
#include "models.hpp"

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

// List.mul：重复自身n次 self * n
inline auto list_mul = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_mul)");
    assert(args->val.size() == 1 && "function List.mul need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_mul must be called by List object");
    
    auto times_int = dynamic_cast<Int*>(args->val[0]);
    assert(times_int != nullptr && "List.mul only supports Int type argument");
    assert(times_int->val >= deps::BigInt(0) && "List.mul requires non-negative integer argument");
    
    std::vector<Object*> new_vals;
    deps::BigInt times = times_int->val;
    for (deps::BigInt i = deps::BigInt(0); i < times; i+=deps::BigInt(1)) {
        new_vals.insert(new_vals.end(), self_list->val.begin(), self_list->val.end());
    }
    
    return new List(std::move(new_vals));
};

// List.eq：判断两个List是否相等
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

// List.contains：判断列表是否包含目标元素
inline auto list_contains = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (list_contains)");
    assert(args->val.size() == 1 && "function List.contains need 1 arg");
    
    auto self_list = dynamic_cast<List*>(self);
    assert(self_list != nullptr && "list_contains must be called by List object");
    
    Object* target_elem = args->val[0];
    assert(target_elem != nullptr && "List.contains target argument cannot be nullptr");
    
    // 遍历列表元素，逐个判断是否与目标元素相等
    for (Object* elem : self_list->val) {
        bool elem_equal = false;
        
        // 按元素类型匹配校验（与list_eq逻辑完全一致，确保行为统一）
        if (auto elem_int = dynamic_cast<Int*>(elem); elem_int) {
            auto target_int = dynamic_cast<Int*>(target_elem);
            elem_equal = (target_int && elem_int->val == target_int->val);
        } else if (auto elem_bool = dynamic_cast<Bool*>(elem); elem_bool) {
            auto target_bool = dynamic_cast<Bool*>(target_elem);
            elem_equal = (target_bool && elem_bool->val == target_bool->val);
        } else if (auto elem_nil = dynamic_cast<Nil*>(elem); elem_nil) {
            auto target_nil = dynamic_cast<Nil*>(target_elem);
            elem_equal = (target_nil != nullptr);
        } else if (auto elem_str = dynamic_cast<String*>(elem); elem_str) {
            auto target_str = dynamic_cast<String*>(target_elem);
            elem_equal = (target_str && elem_str->val == target_str->val);
        }
        
        // 找到匹配元素，立即返回true
        if (elem_equal) {
            return new Bool(true);
        }
    }
    
    // 遍历完未找到匹配元素，返回false
    return new Bool(false);
};

}  // namespace model
