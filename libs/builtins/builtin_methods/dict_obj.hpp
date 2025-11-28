#pragma once
#include "models.hpp"

namespace model {

// Dictionary.add：添加键值对 self + x（key: String，value: 任意Object），返回新Dictionary（不可变语义）
inline auto dict_add = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (dict_add)");
    assert(args->val.size() == 2 && "function Dictionary.add need 2 args: (key: String, value: Object)");
    
    auto self_dict = dynamic_cast<Dictionary*>(self);
    assert(self_dict != nullptr && "dict_add must be called by Dictionary object");
    
    auto key_obj = dynamic_cast<String*>(args->val[0]);
    assert(key_obj != nullptr && "Dictionary.add key must be String type");
    
    Object* value_obj = args->val[1];
    
    // 复制原字典的attrs（返回新字典）
    deps::HashMap<Object*> new_attrs = self_dict->attrs;
    // 插入新键值对
    new_attrs.insert(key_obj->val, value_obj);
    
    return new Dictionary(new_attrs);
};

// Dictionary.contains：x in self 判断是否包含指定键（key: String），返回Bool
inline auto dict_contains = [](Object* self, const List* args) -> Object* {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (dict_contains)");
    assert(args->val.size() == 1 && "function Dictionary.contains need 1 arg: (key: String)");
    
    auto self_dict = dynamic_cast<Dictionary*>(self);
    assert(self_dict != nullptr && "dict_contains must be called by Dictionary object");
    
    // 键必须是String类型
    auto key_obj = dynamic_cast<String*>(args->val[0]);
    assert(key_obj != nullptr && "Dictionary.contains key must be String type");
    
    auto found_node = self_dict->attrs.find_in_current(key_obj->val);
    return new Bool(found_node != nullptr);
};

}  // namespace model