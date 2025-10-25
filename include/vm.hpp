/**
 * @file vm.hpp
 * @brief 虚拟机(VM)核心定义
 * 执行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */
namespace kaz{

struct VmState{
    std::shared_ptr<model::Model> stack_top;
    std::unordered_map<std::string, std::shared_ptr<Model>> locals;
}; 

struct Introduction {
    Opcode opc;
    std::vector<size_t> opn_list;
};

struct CallFrame {
    std::unordered_map<std::string, std::shared_ptr<Model>> locals;
    size_t pc = 0;
    size_t return_to_pc;
    std::string name;
    std::shared_ptr<model::CodeObject> code_object;
};

class Vm {
protect:
    std::stack<std::shared_ptr<model::Model>> op_stack_;
    std::vector<std::shared_ptr<model::Model>>   constant_pool;
    std::stack<std::shared_ptr<CallFrame>> call_stack_;
    size_t pc_;
    std::vector<Introduction> code_list;
    bool running = false;
public:
    void load(std::shared_ptr<model::Module> src_module);
    VmState exec(Introduction introduction);
    };
};