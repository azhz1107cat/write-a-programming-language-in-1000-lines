/**
 * @file ir_gen.hpp
 * @brief 中间代码生成器(IR Generator) 核心定义
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once
#include "ast.hpp"
#include "models.hpp"

#include <memory>
#include <stack>
#include <vector>


namespace kiz {

class IRGenerator {
    std::unique_ptr<ASTNode> ast;
    std::stack<std::string> block_stack;

    std::vector<std::string> curr_names;
    std::vector<Instruction> curr_code_list;
    std::vector<model::Object*> curr_const;
    std::vector<std::tuple<size_t, size_t>> curr_lineno_map;

public:
    model::Module* gen();
    void gen_block(BlockStmt* block);
    void gen_expr();
    void gen_fn_call();
    void gen_dict();
    void gen_fn_decl();
    void gen_if();
    void gen_while();
    void gen_literal();

protected:
    model::CodeObject* make_code_obj();
    model::Int* make_int_obj();
    model::Rational* make_rational_obj();
    model::String* make_string_obj();
};

} // namespace kiz