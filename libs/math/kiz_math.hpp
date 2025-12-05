#pragma once
#include "model.hpp"
#include "../../deps/rational.hpp"

namespace math_lib {

auto pi = new model::Rational(deps::Rational(3.14159));

auto __attrs__ = deps::HashMap();

auto __init_module__ = [](model::Object* self, const model::List* args) -> model::Object* {
    __attrs__.insert("pi", pi);
    return new model::Module(
        "math",
        __attrs__
    );
}

}