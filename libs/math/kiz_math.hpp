#pragma once
#include "model.hpp"
#include "../../deps/rational.hpp"

namespace math_lib {

inline auto pi = new model::Rational(deps::Rational(3.14159));

inline auto __init_module__ = [](model::Object* self, const model::List* args) -> model::Object* {
    auto mod = new model::Module(
        "math",
        nullptr
    );

    mod->attrs.insert("pi", pi);
    
    return mod;
}

}