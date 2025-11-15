#pragma once
#include "models.hpp"

namespace builtin_objects {

inline auto print = [](const model::List* args) -> model::Object* {
    std::string text;
    for (const auto* arg : args->val) {
        text += arg->to_string();
    }
    std::cout << text << std::endl;
    return new model::Nil();
};

}