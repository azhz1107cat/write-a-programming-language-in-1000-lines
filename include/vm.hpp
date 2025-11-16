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

#include "kiz.hpp"
#include "../libs/builtins/builtins.hpp"

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
    deps::HashMap<model::Object*> builtins;

    const std::string& file_path;
public:
    explicit Vm(const std::string& file_path) : file_path(file_path) {
        DEBUG_OUTPUT("registering builtins...");
        builtins.insert("print", new model::CppFunction(builtin_objects::print));
        builtins.insert("input", new model::CppFunction(builtin_objects::input));
    }

    VmState load(const model::Module* src_module);
    void exec(const Instruction& instruction);
    void exec_ADD(const Instruction& instruction);
    void exec_SUB(const Instruction& instruction);
    void exec_MUL(const Instruction& instruction);
    void exec_DIV(const Instruction& instruction);
    void exec_MOD(const Instruction& instruction);
    void exec_POW(const Instruction& instruction);
    void exec_NEG(const Instruction& instruction);
    void exec_EQ(const Instruction& instruction);
    void exec_GT(const Instruction& instruction);
    void exec_LT(const Instruction& instruction);
    void exec_AND(const Instruction& instruction);
    void exec_NOT(const Instruction& instruction);
    void exec_OR(const Instruction& instruction);
    void exec_IS(const Instruction& instruction);
    void exec_IN(const Instruction& instruction) const;
    void exec_MAKE_LIST(const Instruction& instruction);
    void exec_CALL(const Instruction& instruction);
    void exec_RET(const Instruction& instruction);
    void exec_GET_ATTR(const Instruction& instruction);
    void exec_SET_ATTR(const Instruction& instruction);
    void exec_LOAD_VAR(const Instruction& instruction);
    void exec_LOAD_CONST(const Instruction& instruction);
    void exec_SET_GLOBAL(const Instruction& instruction);
    void exec_SET_LOCAL(const Instruction& instruction);
    void exec_SET_NONLOCAL(const Instruction& instruction);
    void exec_JUMP(const Instruction& instruction);
    void exec_JUMP_IF_FALSE(const Instruction& instruction);
    void exec_THROW(const Instruction& instruction);
    void exec_POP_TOP(const Instruction& instruction);
    void exec_SWAP(const Instruction& instruction);
    void exec_COPY_TOP(const Instruction& instruction);
};

} // namespace kiz