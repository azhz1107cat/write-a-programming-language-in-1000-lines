/**
 * @file ir_gen.cpp
 * @brief 中间代码生成器（IR Generator）核心实现
 * 从AST生成IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

model::Module* kiz::IRGenerator::gen() {
    // TODO: 初始化IR生成总流程，协调调用gen_mod解析AST根节点，整合全局常量、代码对象生成最终Module
    return nullptr;
}

void kiz::IRGenerator::gen_mod() {
    // TODO: 解析模块级AST节点，处理全局变量声明、函数声明等顶层元素，填充curr_const、curr_code_list
}

void kiz::IRGenerator::gen_expr() {
    // TODO: 生成表达式IR，处理二元运算、一元运算等表达式节点，生成对应Instruction（如ADD、LOAD_CONST等）
}

void kiz::IRGenerator::gen_fn_call() {
    // TODO: 生成函数调用IR，处理函数名解析、参数压栈、CALL指令生成，维护栈帧状态
}

void kiz::IRGenerator::gen_dict() {
    // TODO: 生成字典类型IR，处理键值对表达式解析，生成BUILD_DICT指令，将字典对象加入curr_const
}

void kiz::IRGenerator::gen_fn_decl() {
    // TODO: 生成函数声明IR，处理函数参数列表、函数体块（调用gen_block），通过make_code_obj生成函数对应的CodeObject
}

void kiz::IRGenerator::gen_if() {
    // TODO: 生成条件分支IR，处理条件表达式（gen_expr）、if块/else块跳转，生成JUMP_IF_FALSE、JUMP等指令
}

void kiz::IRGenerator::gen_while() {
    // TODO: 生成循环IR，处理循环条件（gen_expr）、循环体块，生成JUMP_BACK、JUMP_IF_FALSE等指令维护循环逻辑
}

void kiz::IRGenerator::gen_literal() {
    // TODO: 生成字面量IR，区分整数、小数、字符串等类型，调用对应make_xxx_obj函数加入curr_const，生成LOAD_CONST指令
}

// -------------------------- Protected 成员函数实现 --------------------------
model::CodeObject* kiz::IRGenerator::make_code_obj() {
    // TODO: 封装当前代码块信息（curr_code_list指令集、curr_names变量名表、curr_lineno_map行号映射），生成CodeObject实例
    return nullptr;
}

model::Int* kiz::IRGenerator::make_int_obj() {
    // TODO: 从AST字面量节点提取整数值，创建并初始化model::Int对象，加入curr_const管理
    return nullptr;
}

model::Dec* kiz::IRGenerator::make_dec_obj() {
    // TODO: 从AST字面量节点提取小数值（如float/double），创建并初始化model::Dec对象，加入curr_const管理
    return nullptr;
}

model::Rational* kiz::IRGenerator::make_rational_obj() {
    // TODO: 从AST字面量节点提取有理数（如分数形式），创建并初始化model::Rational对象，加入curr_const管理
    return nullptr;
}

model::String* kiz::IRGenerator::make_string_obj() {
    // TODO: 从AST字面量节点提取字符串值，处理转义字符，创建并初始化model::String对象，加入curr_const管理
    return nullptr;
}