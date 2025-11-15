/**
 * @file vm.hpp
 * @brief 虚拟机(VM)核心定义
 * 执行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */
#pragma once

#include "../deps/bigint.hpp"
#include "../deps/hashmap.hpp"

#include <stack>

namespace model {
class Module;
class CodeObject;
class Object;
}

namespace kiz {

enum class Opcode;

struct VmState{
    model::Object* stack_top;
    deps::HashMap<model::Object*> locals;
}; 

struct Instruction {
    Opcode opc;
    std::vector<size_t> opn_list;
    size_t start_lineno;
    size_t end_lineno;
};

struct CallFrame {
    bool is_week_scope;
    deps::HashMap<model::Object*> locals;
    size_t pc = 0;
    size_t return_to_pc;
    std::string name;
    model::CodeObject* code_object;
    std::vector<std::tuple<size_t, size_t>> curr_lineno_map;
    std::vector<std::string> names;
};

class Vm {
    std::stack<model::Object *> op_stack_;
    std::vector<model::Object*> constant_pool_;
    std::vector<std::unique_ptr<CallFrame>> call_stack_;
    size_t pc_ = 0;
    std::vector<Instruction> code_list_;
    bool running_ = false;

    const std::string& file_path;
public:
    explicit Vm(const std::string& file_path) : file_path(file_path) {}

    VmState load(const model::Module* src_module);
    void exec(Instruction introduction);
};

} // namespace kiz