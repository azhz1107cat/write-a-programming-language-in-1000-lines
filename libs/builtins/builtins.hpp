#pragma once
#include "models.hpp"

inline model::Object* get_one_arg(const model::List* args) {
    if (!args->val.empty()) {
        return args->val[0];
    }
    assert(false && "函数参数不足一个");
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

}