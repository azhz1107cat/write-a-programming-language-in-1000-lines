#include "../include/models.hpp"

namespace model {

void registering_std_modules() {
    std_modules.insert("math", new CppFunction(
        math_lib::__init_module__
    ));
}

} // namespace model