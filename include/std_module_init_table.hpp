#pragma once
#include "hashmap.hpp"
#include "models.hpp"

namespace kiz {

inline deps::HashMap<model::CppFunction*> std_module_init_table;

inline void registering_std_module_init_table() {
    std_module_init_table.insert("math", nullptr);
}

}
