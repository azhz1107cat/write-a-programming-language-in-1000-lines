/**
 * @file vm.hpp
 * @brief 虚拟机(VM)核心定义
 * 执行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */
#pragma once

#include "../deps/hashmap.hpp"

#include <stack>
#include <tuple>

#include "kiz.hpp"
#include "models.hpp"
#include "../libs/builtins/builtins.hpp"


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
    // deps::HashMap<model::Module*> loaded_modules;
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
        #define KIZ_FUNC(n) builtins.insert(#n, new model::CppFunction(builtin_objects::n))
        KIZ_FUNC(print);
        KIZ_FUNC(input);
        KIZ_FUNC(isintance);
        #undef KIZ_FUNC

        DEBUG_OUTPUT("registering magic methods...");
        model::registering_magic_methods();
    }

    void load(model::Module* src_module);
    void extend_code(const model::CodeObject* code_object);
    VmState get_vm_state();
    void exec(const Instruction& instruction);
    std::tuple<model::Object*, model::Object*> fetch_two_from_stack_top(const std::string& curr_instruction_name);
    static bool check_has_magic(model::Object* a, const std::string& magic_method_name);
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
    void exec_IN(const Instruction& instruction);
    void exec_MAKE_LIST(const Instruction& instruction);
    void call_function(model::Object* func_obj, model::Object* args_obj, model::Object* self);
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