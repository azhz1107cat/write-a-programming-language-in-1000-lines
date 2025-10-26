/**
 * @file vm.hpp
 * @brief 虚拟机(VM)核心定义
 * 执行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */
namespace kiz{

struct VmState{
    model::Object* stack_top;
    deps::HashMap<std::string, model::Object*> locals;
}; 

struct Instruction {
    Opcode opc;
    std::vector<size_t> opn_list;
};

struct CallFrame {
    deps::HashMap<std::string, model::Object*> locals;
    size_t pc = 0;
    size_t return_to_pc;
    std::string name;
    model::CodeObject* code_object;
    std::vector<std::tuple<size_t, size_t>> curr_lineno_map;
    std::vector<std::string> names;
};

class Vm {
    std::stack<model::Object*> op_stack_;
    std::vector<model::Object*> constant_pool;
    std::stack<std::unique_ptr<CallFrame>> call_stack_;
    size_t pc_;
    std::vector<Instruction> code_list;
    bool running = false;

public:
    void load(model::Module* src_module);
    VmState exec(std::vector<Introduction> introduction);

protected:
    void eval_call();
    void eval_set_member();
    void eval_ret();
};

} // namespace kiz