/**
 * @file vm.cpp
 * @brief 虚拟机（VM）核心实现
 * 用于运行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include <cassert>
#include <algorithm>

namespace kiz {

void Vm::load(model::Module* src_module) {
    // 合法性校验：防止空指针访问
    assert(src_module != nullptr && "Vm::load: 传入的src_module不能为nullptr");
    assert(src_module->code != nullptr && "Vm::load: 模块的CodeObject未初始化（code为nullptr）");

    // 加载指令列表：将模块CodeObject中的指令复制到VM的执行指令池
    this->code_list_ = src_module->code->code;

    // 加载常量池：处理引用计数，避免常量对象被提前释放
    const std::vector<model::Object*>& module_consts = src_module->code->consts;
    for (model::Object* const_obj : module_consts) {
        assert(const_obj != nullptr && "Vm::load: 常量池中有nullptr对象，非法");
        const_obj->make_ref(); // 增加引用计数（VM持有常量的引用）
        this->constant_pool_.push_back(const_obj);
    }

    // 创建模块级调用帧（CallFrame）：模块是顶层执行单元，对应一个顶层调用帧
    auto module_call_frame = std::make_unique<CallFrame>();
    module_call_frame->is_week_scope = false;          // 模块作用域为"强作用域"（非弱作用域）
    module_call_frame->locals = deps::HashMap<std::string, model::Object*>(); // 初始空局部变量表
    module_call_frame->pc = 0;                         // 程序计数器初始化为0（从第一条指令开始执行）
    module_call_frame->return_to_pc = this->code_list_.size(); // 执行完所有指令后返回的位置（指令池末尾）
    module_call_frame->name = src_module->name;        // 调用帧名称与模块名一致（便于调试）
    module_call_frame->code_object = src_module->code; // 关联当前模块的CodeObject
    module_call_frame->curr_lineno_map = src_module->code->lineno_map; // 复制行号映射（用于错误定位）
    module_call_frame->names = src_module->code->names; // 复制变量名列表（指令操作数索引对应此列表）

    // 将调用帧压入VM的调用栈
    this->call_stack_.push(std::move(module_call_frame));

    // 初始化VM执行状态：标记为"就绪"
    this->pc_ = 0;         // 全局PC同步为调用帧初始PC
    this->running_ = true; // 标记VM为运行状态（等待exec触发执行）
}

VmState Vm::exec(Instruction instruction) { 
    switch (instruction.opc) {
        case Opcode::OP_ADD: {
            // 二元运算：至少需要2个操作数
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_ADD: 操作数栈元素不足（需≥2）");
            }
            // 弹出栈顶2个操作数（注意：栈是LIFO，先弹右操作数）
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            // 仅支持Int类型（可扩展Rational），通过dynamic_cast校验类型
            model::Int* a_int = dynamic_cast<model::Int*>(a);
            model::Int* b_int = dynamic_cast<model::Int*>(b);
            if (!a_int || !b_int) {
                std::assert(false && "OP_ADD: 仅支持Int类型运算");
            }

            // 计算并压入结果（注意引用计数管理）
            model::Int* result = new model::Int();
            result->val = a_int->val + b_int->val;  // BigInt支持+运算符
            result->make_ref();  // 增加引用计数，避免内存泄漏
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_SUB: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_SUB: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Int* a_int = dynamic_cast<model::Int*>(a);
            model::Int* b_int = dynamic_cast<model::Int*>(b);
            if (!a_int || !b_int) {
                std::assert(false && "OP_SUB: 仅支持Int类型运算");
            }

            model::Int* result = new model::Int();
            result->val = a_int->val - b_int->val;
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_MUL: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_MUL: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Int* a_int = dynamic_cast<model::Int*>(a);
            model::Int* b_int = dynamic_cast<model::Int*>(b);
            if (!a_int || !b_int) {
                std::assert(false && "OP_MUL: 仅支持Int类型运算");
            }

            model::Int* result = new model::Int();
            result->val = a_int->val * b_int->val;
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_DIV: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_DIV: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Int* a_int = dynamic_cast<model::Int*>(a);
            model::Int* b_int = dynamic_cast<model::Int*>(b);
            if (!a_int || !b_int) {
                std::assert(false && "OP_DIV: 仅支持Int类型运算");
            }
            // 校验除数不为0（BigInt假设支持==）
            if (b_int->val == deps::BigInt(0)) {
                std::assert(false && "OP_DIV: 除数不能为0");
            }

            model::Int* result = new model::Int();
            result->val = a_int->val / b_int->val;  // 整数除法（可扩展为Rational浮点）
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_MOD: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_MOD: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Int* a_int = dynamic_cast<model::Int*>(a);
            model::Int* b_int = dynamic_cast<model::Int*>(b);
            if (!a_int || !b_int) {
                std::assert(false && "OP_MOD: 仅支持Int类型运算");
            }
            if (b_int->val == deps::BigInt(0)) {
                std::assert(false && "OP_MOD: 除数不能为0");
            }

            model::Int* result = new model::Int();
            result->val = a_int->val % b_int->val;
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_POW: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_POW: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();  // 指数
            op_stack_.pop();
            model::Object* a = op_stack_.top();  // 底数
            op_stack_.pop();

            model::Int* a_int = dynamic_cast<model::Int*>(a);
            model::Int* b_int = dynamic_cast<model::Int*>(b);
            if (!a_int || !b_int) {
                std::assert(false && "OP_POW: 仅支持Int类型运算");
            }

            model::Int* result = new model::Int();
            result->val = a_int->val.pow(b_int->val.to_uint64());  // 假设BigInt有pow方法
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_NEG: {
            // 一元运算：仅需1个操作数
            if (op_stack_.empty()) {
                std::assert(false && "OP_NEG: 操作数栈为空");
            }
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Int* a_int = dynamic_cast<model::Int*>(a);
            if (!a_int) {
                std::assert(false && "OP_NEG: 仅支持Int类型运算");
            }

            model::Int* result = new model::Int();
            result->val = -a_int->val;  // 取负
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_EQ: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_EQ: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            bool is_equal = false;
            // 处理Int类型比较
            if (model::Int* a_int = dynamic_cast<model::Int*>(a); a_int) {
                if (model::Int* b_int = dynamic_cast<model::Int*>(b); b_int) {
                    is_equal = (a_int->val == b_int->val);
                }
            }
            // 处理Nil类型（Nil == Nil 为true）
            else if (dynamic_cast<model::Nil*>(a) && dynamic_cast<model::Nil*>(b)) {
                is_equal = true;
            }
            // 处理String类型（扩展）
            else if (model::String* a_str = dynamic_cast<model::String*>(a); a_str) {
                if (model::String* b_str = dynamic_cast<model::String*>(b); b_str) {
                    is_equal = (a_str->val == b_str->val);
                }
            }
            else {
                std::assert(false && "OP_EQ: 不支持的类型比较");
            }

            // 压入Bool结果（假设Bool有静态实例优化，此处简化为new）
            model::Bool* result = new model::Bool();
            result->val = is_equal;  // 假设Bool有bool类型成员val
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_GT: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_GT: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();  // 右操作数（a > b）
            op_stack_.pop();
            model::Object* a = op_stack_.top();  // 左操作数
            op_stack_.pop();

            bool is_gt = false;
            if (model::Int* a_int = dynamic_cast<model::Int*>(a); a_int) {
                if (model::Int* b_int = dynamic_cast<model::Int*>(b); b_int) {
                    is_gt = (a_int->val > b_int->val);
                }
            }
            else {
                std::assert(false && "OP_GT: 仅支持Int类型比较");
            }

            model::Bool* result = new model::Bool();
            result->val = is_gt;
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_LT: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_LT: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();  // 右操作数（a < b）
            op_stack_.pop();
            model::Object* a = op_stack_.top();  // 左操作数
            op_stack_.pop();

            bool is_lt = false;
            if (model::Int* a_int = dynamic_cast<model::Int*>(a); a_int) {
                if (model::Int* b_int = dynamic_cast<model::Int*>(b); b_int) {
                    is_lt = (a_int->val < b_int->val);
                }
            }
            else {
                std::assert(false && "OP_LT: 仅支持Int类型比较");
            }

            model::Bool* result = new model::Bool();
            result->val = is_lt;
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_AND: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_AND: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Bool* a_bool = dynamic_cast<model::Bool*>(a);
            model::Bool* b_bool = dynamic_cast<model::Bool*>(b);
            if (!a_bool || !b_bool) {
                std::assert(false && "OP_AND: 仅支持Bool类型运算");
            }

            model::Bool* result = new model::Bool();
            result->val = (a_bool->val && b_bool->val);  // 逻辑与
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_NOT: {
            if (op_stack_.empty()) {
                std::assert(false && "OP_NOT: 操作数栈为空");
            }
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Bool* a_bool = dynamic_cast<model::Bool*>(a);
            if (!a_bool) {
                std::assert(false && "OP_NOT: 仅支持Bool类型运算");
            }

            model::Bool* result = new model::Bool();
            result->val = !a_bool->val;  // 逻辑非
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_OR: {
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_OR: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            model::Bool* a_bool = dynamic_cast<model::Bool*>(a);
            model::Bool* b_bool = dynamic_cast<model::Bool*>(b);
            if (!a_bool || !b_bool) {
                std::assert(false && "OP_OR: 仅支持Bool类型运算");
            }

            model::Bool* result = new model::Bool();
            result->val = (a_bool->val || b_bool->val);  // 逻辑或
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_IS: {
            // 判断两个对象是否为同一实例（地址比较）
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_IS: 操作数栈元素不足（需≥2）");
            }
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            model::Object* a = op_stack_.top();
            op_stack_.pop();

            bool is_same = (a == b);  // 直接比较对象地址
            model::Bool* result = new model::Bool();
            result->val = is_same;
            result->make_ref();
            op_stack_.push(result);
            break;
        }

        case Opcode::OP_IN: {
            // 判断元素是否在容器中
            if (op_stack_.size() < 2) {
                std::assert(false && "OP_IN: 操作数栈元素不足（需≥2）");
            }
            // ToDo:...
            break;
        }

        case Opcode::CALL: {
            if (op_stack_.empty() || call_stack_.empty()) {
                std::assert(false && "CALL: 操作数栈为空或无活跃调用帧");
            }
            // 栈顶：函数对象；栈顶下n个元素：函数参数（n=func->argc）
            model::Object* func_obj = op_stack_.top();
            op_stack_.pop();
            model::Function* func = dynamic_cast<model::Function*>(func_obj);
            if (!func) {
                std::assert(false && "CALL: 栈顶元素非Function类型");
            }
            size_t required_argc = func->argc;  // 函数声明的参数个数

            // 校验参数数量
            if (op_stack_.size() < required_argc) {
                std::assert(false && "CALL: 实际参数数量不足");
            }

            // 创建新调用帧（CallFrame）
            auto new_frame = std::make_unique<CallFrame>();
            new_frame->name = func->name;
            new_frame->code_object = func->code;  // 函数关联的字节码对象
            new_frame->pc = 0;                    // 函数内程序计数器初始化为0
            new_frame->return_to_pc = pc_;        // 保存当前pc作为返回地址
            new_frame->names = func->code->names; // 函数的局部变量名表
            new_frame->is_week_scope = false;     // 默认为强作用域（可按语义调整）

            // 从操作数栈弹出参数，存入新调用帧的locals
            for (size_t i = 0; i < required_argc; ++i) {
                if (i >= new_frame->names.size()) {
                    std::assert(false && "CALL: 参数名索引超出范围");
                }
                std::string param_name = new_frame->names[i];  // 假设names前n个为参数名
                model::Object* param_val = op_stack_.top();
                op_stack_.pop();
                param_val->make_ref();  // 增加引用计数
                new_frame->locals[param_name] = param_val;
            }

            // 压入新调用帧，更新pc为函数字节码起始位置
            call_stack_.push(std::move(new_frame));
            pc_ = 0;
            break;
        }

        case Opcode::RET: {
            // 函数返回：弹出当前调用帧，恢复调用者pc，返回值压入调用者栈
            if (call_stack_.size() < 2) {
                std::assert(false && "RET: 无调用者（顶层调用帧无法返回）");
            }
            // 弹出当前调用帧（ ownership转移）
            std::unique_ptr<CallFrame> curr_frame = std::move(call_stack_.top());
            call_stack_.pop();
            CallFrame* caller_frame = call_stack_.top().get();  // 调用者帧

            // 处理返回值（栈顶为返回值，无则默认Nil）
            model::Object* return_val = new model::Nil();
            return_val->make_ref();
            if (!op_stack_.empty()) {
                return_val->del_ref();  // 释放默认Nil的引用
                return_val = op_stack_.top();
                op_stack_.pop();
                return_val->make_ref();
            }

            // 恢复调用者的程序计数器
            pc_ = curr_frame->return_to_pc;

            // 将返回值压入调用者的操作数栈
            op_stack_.push(return_val);
            break;
        }

        case Opcode::GET_ATTR: {
            // 栈顶：对象；opn_list[0]：属性名在names中的索引
            if (op_stack_.empty() || instruction.opn_list.empty()) {
                std::assert(false && "GET_ATTR: 操作数栈为空或无属性名索引");
            }
            model::Object* obj = op_stack_.top();
            op_stack_.pop();
            size_t name_idx = instruction.opn_list[0];
            CallFrame* curr_frame = call_stack_.top().get();

            // 校验属性名索引
            if (name_idx >= curr_frame->names.size()) {
                std::assert(false && "GET_ATTR: 属性名索引超出范围");
            }
            std::string attr_name = curr_frame->names[name_idx];

            // 从对象的attrs中查找属性
            auto attr_it = obj->attrs.find(attr_name);
            if (attr_it == obj->attrs.end()) {
                std::assert(false && "GET_ATTR: 对象无此属性");
            }
            model::Object* attr_val = attr_it->second;
            attr_val->make_ref();
            op_stack_.push(attr_val);
            break;
        }

        case Opcode::SET_ATTR: {
            // 栈顶：属性值；栈顶下：对象；opn_list[0]：属性名索引
            if (op_stack_.size() < 2 || instruction.opn_list.empty()) {
                std::assert(false && "SET_ATTR: 操作数栈元素不足或无属性名索引");
            }
            model::Object* attr_val = op_stack_.top();
            op_stack_.pop();
            model::Object* obj = op_stack_.top();
            op_stack_.pop();
            size_t name_idx = instruction.opn_list[0];
            CallFrame* curr_frame = call_stack_.top().get();

            if (name_idx >= curr_frame->names.size()) {
                std::assert(false && "SET_ATTR: 属性名索引超出范围");
            }
            std::string attr_name = curr_frame->names[name_idx];

            // 更新属性（旧值减引用，新值加引用）
            auto attr_it = obj->attrs.find(attr_name);
            if (attr_it != obj->attrs.end()) {
                attr_it->second->del_ref();  // 释放旧值引用
            }
            attr_val->make_ref();
            obj->attrs[attr_name] = attr_val;
            break;
        }

        case Opcode::LOAD_VAR: {
            // 从当前调用帧的locals加载变量（opn_list[0]：变量名索引）
            if (call_stack_.empty() || instruction.opn_list.empty()) {
                std::assert(false && "LOAD_VAR: 无调用帧或无变量名索引");
            }
            CallFrame* curr_frame = call_stack_.top().get();
            size_t name_idx = instruction.opn_list[0];
            if (name_idx >= curr_frame->names.size()) {
                std::assert(false && "LOAD_VAR: 变量名索引超出范围");
            }
            std::string var_name = curr_frame->names[name_idx];

            // 查找变量
            auto var_it = curr_frame->locals.find(var_name);
            if (var_it == curr_frame->locals.end()) {
                std::assert(false && "LOAD_VAR: 局部变量未定义");
            }
            model::Object* var_val = var_it->second;
            var_val->make_ref();
            op_stack_.push(var_val);
            break;
        }

        case Opcode::LOAD_CONST: {
            // 从常量池加载常量（opn_list[0]：常量索引）
            if (instruction.opn_list.empty()) {
                std::assert(false && "LOAD_CONST: 无常量索引");
            }
            size_t const_idx = instruction.opn_list[0];
            if (const_idx >= constant_pool_.size()) {
                std::assert(false && "LOAD_CONST: 常量索引超出范围");
            }
            model::Object* const_val = constant_pool_[const_idx];
            const_val->make_ref();
            op_stack_.push(const_val);
            break;
        }

        case Opcode::SET_GLOBAL: {
            // 存储全局变量（顶层调用帧的locals，通常是模块级）
            if (call_stack_.empty() || op_stack_.empty() || instruction.opn_list.empty()) {
                std::assert(false && "SET_GLOBAL: 无调用帧/栈空/无变量名索引");
            }
            // 全局变量存于最顶层调用帧（模块帧）
            CallFrame* global_frame = call_stack_.front().get();
            size_t name_idx = instruction.opn_list[0];
            if (name_idx >= global_frame->names.size()) {
                std::assert(false && "SET_GLOBAL: 变量名索引超出范围");
            }
            std::string var_name = global_frame->names[name_idx];

            model::Object* var_val = op_stack_.top();
            op_stack_.pop();
            var_val->make_ref();

            // 更新全局变量（旧值减引用）
            auto var_it = global_frame->locals.find(var_name);
            if (var_it != global_frame->locals.end()) {
                var_it->second->del_ref();
            }
            global_frame->locals[var_name] = var_val;
            break;
        }

        case Opcode::SET_LOCAL: {
            // 存储局部变量（当前调用帧的locals）
            if (call_stack_.empty() || op_stack_.empty() || instruction.opn_list.empty()) {
                std::assert(false && "SET_LOCAL: 无调用帧/栈空/无变量名索引");
            }
            CallFrame* curr_frame = call_stack_.top().get();
            size_t name_idx = instruction.opn_list[0];
            if (name_idx >= curr_frame->names.size()) {
                std::assert(false && "SET_LOCAL: 变量名索引超出范围");
            }
            std::string var_name = curr_frame->names[name_idx];

            model::Object* var_val = op_stack_.top();
            op_stack_.pop();
            var_val->make_ref();

            // 更新局部变量
            auto var_it = curr_frame->locals.find(var_name);
            if (var_it != curr_frame->locals.end()) {
                var_it->second->del_ref();
            }
            curr_frame->locals[var_name] = var_val;
            break;
        }

        case Opcode::SET_NONLOCAL: {
            // 存储非局部变量（向上查找非当前的局部作用域）
            if (call_stack_.size() < 2 || op_stack_.empty() || instruction.opn_list.empty()) {
                std::assert(false && "SET_NONLOCAL: 调用帧不足/栈空/无变量名索引");
            }
            size_t name_idx = instruction.opn_list[0];
            std::string var_name;
            CallFrame* target_frame = nullptr;

            // 从当前帧的上一帧开始查找（跳过当前帧）
            auto frame_it = call_stack_.rbegin();
            ++frame_it;  // rbegin()是当前帧，++后是上一帧
            for (; frame_it != call_stack_.rend(); ++frame_it) {
                CallFrame* frame = frame_it->get();
                if (name_idx >= frame->names.size()) continue;
                var_name = frame->names[name_idx];
                if (frame->locals.count(var_name)) {
                    target_frame = frame;
                    break;
                }
            }

            if (!target_frame) {
                std::assert(false && "SET_NONLOCAL: 未找到非局部变量");
            }

            model::Object* var_val = op_stack_.top();
            op_stack_.pop();
            var_val->make_ref();

            // 更新目标作用域变量
            auto var_it = target_frame->locals.find(var_name);
            if (var_it != target_frame->locals.end()) {
                var_it->second->del_ref();
            }
            target_frame->locals[var_name] = var_val;
            break;
        }

        case Opcode::JUMP: {
            // 无条件跳转到opn_list[0]指定的pc
            if (instruction.opn_list.empty()) {
                std::assert(false && "JUMP: 无目标pc索引");
            }
            size_t target_pc = instruction.opn_list[0];
            if (target_pc >= code_list_.size()) {
                std::assert(false && "JUMP: 目标pc超出字节码范围");
            }
            pc_ = target_pc;  // 直接修改程序计数器
            break;
        }

        case Opcode::JUMP_IF_FALSE: {
            // 栈顶为条件，为假（Nil/Bool(false)）则跳转
            if (op_stack_.empty() || instruction.opn_list.empty()) {
                std::assert(false && "JUMP_IF_FALSE: 栈空/无目标pc");
            }
            model::Object* cond = op_stack_.top();
            op_stack_.pop();
            size_t target_pc = instruction.opn_list[0];

            bool need_jump = false;
            if (dynamic_cast<model::Nil*>(cond)) {
                need_jump = true;
            }
            else if (model::Bool* cond_bool = dynamic_cast<model::Bool*>(cond); cond_bool) {
                need_jump = !cond_bool->val;
            }
            else {
                std::assert(false && "JUMP_IF_FALSE: 条件必须是Nil或Bool");
            }

            if (need_jump) {
                if (target_pc >= code_list_.size()) {
                    std::assert(false && "JUMP_IF_FALSE: 目标pc超出范围");
                }
                pc_ = target_pc;
            }
            break;
        }

        case Opcode::THROW: {
            break;
        }

        case Opcode::POP_TOP: {
            if (!op_stack_.empty()) {
                model::Object* top = op_stack_.top();
                op_stack_.pop();
                top->del_ref();  // 补充：弹出时减少引用计数，避免内存泄漏
            } else {
                std::assert(false && "POP_TOP: 操作数栈为空");
            }
            break;
        }

        case Opcode::SWAP: {
            // 交换栈顶两个元素
            if (op_stack_.size() < 2) {
                std::assert(false && "SWAP: 操作数栈元素不足（需≥2）");
            }
            model::Object* a = op_stack_.top();
            op_stack_.pop();
            model::Object* b = op_stack_.top();
            op_stack_.pop();
            // 按原顺序压回（交换后a在栈顶）
            op_stack_.push(a);
            op_stack_.push(b);
            break;
        }

        case Opcode::COPY_TOP: {
            // 复制栈顶元素并压入
            if (op_stack_.empty()) {
                std::assert(false && "COPY_TOP: 操作数栈为空");
            }
            model::Object* top = op_stack_.top();
            top->make_ref();  // 复制需增加引用计数
            op_stack_.emplace(top);
            break;
        }

        default:
            std::assert(false && "exec: 未知 opcode");
            break;
    }

    // 构造并返回当前虚拟机状态
    VmState state;
    // 栈顶：操作数栈非空则为栈顶元素，否则为nullptr
    state.stack_top = op_stack_.empty() ? nullptr : op_stack_.top();
    // 局部变量：当前调用帧的locals，无调用帧则为空
    state.locals = call_stack_.empty() 
        ? deps::HashMap<std::string, model::Object*>() 
        : call_stack_.top()->locals;
    return state;
}

} // namespace kiz