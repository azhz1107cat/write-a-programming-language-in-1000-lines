#pragma once
#include <cassert>
#include <unordered_set>

#include "models.hpp"

inline model::Object* get_one_arg(const model::List* args) {
    if (!args->val.empty()) {
        return args->val[0];
    }
    assert(false && "函数参数不足一个");
}

inline model::Object* check_based_object_inner(
    model::Object* src_obj, 
    model::Object* for_check_obj,
    std::unordered_set<model::Object*>& visited
) {
    if (src_obj == nullptr) return nullptr;
    // 闭环检测
    if (visited.contains(src_obj)) return new model::Bool(false);
    visited.insert(src_obj);

    // 查找__parent__属性
    const auto it = src_obj->attrs.find("__parent__");
    if (it == nullptr) {
        return new model::Bool(false);
    }
    // 找到目标返回true，否则递归检查父对象
    if (it->value == for_check_obj) return new model::Bool(true);
    return check_based_object_inner(it->value, for_check_obj, visited);
}

// 对外接口
inline model::Object* check_based_object(model::Object* src_obj, model::Object* for_check_obj) {
    std::unordered_set<model::Object*> visited; // 每次调用新建集合
    return check_based_object_inner(src_obj, for_check_obj, visited);
}

namespace builtin_objects {

inline auto print = [](model::Object* self, const model::List* args) -> model::Object* {
    std::string text;
    for (const auto* arg : args->val) {
        text += arg->to_string() + " ";
    }
    std::cout << text << std::endl;
    return new model::Nil();
};

inline auto input = [](model::Object* self, const model::List* args) -> model::Object* {
    const auto prompt_obj = get_one_arg(args);
    std::cout << prompt_obj->to_string();
    std::string result;
    std::getline(std::cin, result);
    return new model::String(result);
};

inline auto isinstance = [](model::Object* self, const model::List* args) -> model::Object* {
    if (!(args->val.size() == 2)) {
        assert(false && "函数参数不足两个");
    }

    const auto a = args->val[0];
    const auto b = args->val[1];
    return check_based_object(a, b);
    
};

}