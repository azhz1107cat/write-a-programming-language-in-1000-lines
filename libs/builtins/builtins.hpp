#pragma once
#include "models.hpp"

inline model::Object* get_one_arg(const model::List* args) {
    if (!args->val.empty()) {
        return args->val[0];
    }
    assert(false && "函数参数不足一个");
}

inline model::Object* find_based_object(model::Object* src_obj) {
    if (src_obj == nullptr) return nullptr;
    auto it = src_obj->attrs.find("__parent__");
    if (it == nullptr) {
        return src_obj;
    }
    return find_based_object(it->value);
}

namespace builtin_objects {

inline auto print = [](model::Object* self, const model::List* args) -> model::Object* {
    std::string text;
    for (const auto* arg : args->val) {
        text += arg->to_string();
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
    if (!args.size() == 2) {
        assert(false && "函数参数不足两个");
    }

    const auto a = args[0];
    const auto b = args[1];
    // todo: find_based_object(a) 
    
};

}