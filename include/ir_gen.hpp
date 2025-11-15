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
    std::unique_ptr<BlockStmt> ast;
    std::stack<size_t> block_stack;

    std::vector<std::string> curr_names;
    std::vector<Instruction> curr_code_list;
    std::vector<model::Object*> curr_const;
    std::vector<std::tuple<size_t, size_t>> curr_lineno_map;

    const std::string& file_path;
public:
    explicit IRGenerator(const std::string& file_path) : file_path(file_path) {}
    model::Module* gen(std::unique_ptr<BlockStmt> ast_into);

    static size_t get_or_add_name(std::vector<std::string>& names, const std::string& name);
    static size_t get_or_add_const(std::vector<model::Object*>& consts, model::Object* obj);

    [[nodiscard]] model::Module* gen_mod(const std::string& module_name) const;
    void gen_block(const BlockStmt* block);
    void gen_fn_decl(FuncDefStmt* fn_decl);

    void gen_literal(Expression* expr);
    void gen_fn_call(CallExpr* expr);
    void gen_dict(DictDeclExpr* expr);
    void gen_expr(Expression* expr);

    void gen_if(IfStmt* if_stmt);
    void gen_while(WhileStmt* while_stmt);

protected:
    [[nodiscard]] model::CodeObject* make_code_obj() const;
    static model::Int* make_int_obj(const NumberExpr* num_expr);
    static model::Rational* make_rational_obj(NumberExpr* num_expr);
    static model::String* make_string_obj(const StringExpr* str_expr);
};

} // namespace kiz