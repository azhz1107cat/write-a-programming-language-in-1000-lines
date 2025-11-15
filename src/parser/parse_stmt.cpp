#include "parser.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include "ui/color.hpp"

namespace kiz {
// parse_block
std::unique_ptr<BlockStmt> Parser::parse_block() {
    std::vector<std::unique_ptr<Statement>> block_stmts;
    // 循环解析块内语句，直到遇到end关键字（替代原RBrace）
    while (curr_tok_idx_ < tokens_.size()) {
        const Token& curr_tok = curr_token();
        if (curr_tok.type == TokenType::EndOfFile) {
            break;  // 遇到end，结束块解析
        }
        // 解析单条语句并加入块
        if (auto stmt = parse_stmt(); stmt != nullptr) {
            block_stmts.push_back(std::move(stmt));
        }
        // 跳过语句后的分号（若存在）
        if (curr_token().type == TokenType::Semicolon) {
            skip_token(";");
        }
    }
    // 校验块结束符是否为end（防止提前EOF）
    const Token& end_tok = curr_token();
    if (end_tok.type != TokenType::EndOfFile) {
        // std::cerr << Color::RED
        //           << "[Syntax Error] Block must end with 'end', got '"
        //           << end_tok.text << "' (Line: " << end_tok.line << ", Col: " << end_tok.col << ")"
        //           << Color::RESET << std::endl;
        // assert(false && "Block missing 'end' terminator");
    }
    // 跳过end关键字，完成块解析
    skip_token("end");
    return std::make_unique<BlockStmt>(std::move(block_stmts));
}

// parse_if实现
std::unique_ptr<IfStmt> Parser::parse_if() {
    // 解析if条件表达式
    auto cond_expr = parse_expression();
    if (!cond_expr) {
        std::cerr << Color::RED
                  << "[Syntax Error] If statement missing condition expression"
                  << Color::RESET << std::endl;
        assert(false && "Invalid if condition");
    }
    // 解析if体（无大括号，直接调用parse_block）
    skip_start_of_block();  // 跳过条件后的换行
    auto if_block = parse_block();
    // 处理else分支（默认空）
    std::unique_ptr<BlockStmt> else_block = nullptr;
    if (curr_token().type == TokenType::Else) {
        skip_token("else");
        if (curr_token().type == TokenType::If) {
            // else if分支：递归解析if语句，包装为BlockStmt
            std::vector<std::unique_ptr<Statement>> else_if_stmts;
            else_if_stmts.push_back(parse_if());
            else_block = std::make_unique<BlockStmt>(std::move(else_if_stmts));
        } else {
            // else分支：直接解析块
            skip_start_of_block();
            else_block = parse_block();
        }
    }
    return std::make_unique<IfStmt>(std::move(cond_expr), std::move(if_block), std::move(else_block));
}

// parse_stmt实现
std::unique_ptr<Statement> Parser::parse_stmt() {
    const Token curr_tok = curr_token();

    // 解析if语句
    if (curr_tok.type == TokenType::If) {
        skip_token("if");
        return parse_if();  // 复用parse_if逻辑
    }

    // 解析while语句（适配end结尾）
    if (curr_tok.type == TokenType::While) {
        skip_token("while");
        // 解析循环条件表达式
        auto cond_expr = parse_expression();
        if (!cond_expr) {
            // std::cerr << Color::RED
            //           << "[Syntax Error] While statement missing condition expression"
            //           << Color::RESET << std::endl;
            // assert(false && "Invalid while condition");
        }
        // 解析循环体（无大括号，用end结尾）
        skip_start_of_block();
        auto while_block = parse_block();
        return std::make_unique<WhileStmt>(std::move(cond_expr), std::move(while_block));
    }

    // 解析函数定义（新语法：fn x() end）
    if (curr_tok.type == TokenType::Func) {
        skip_token("func");
        // 读取函数名（必须是标识符）
        const Token func_name_tok = skip_token();
        if (func_name_tok.type != TokenType::Identifier) {
            // std::cerr << Color::RED
            //           << "[Syntax Error] Function name must be an identifier, got '"
            //           << func_name_tok.text << "' (Line: " << func_name_tok.line << ")"
            //           << Color::RESET << std::endl;
            // assert(false && "Invalid function name");
        }
        const std::string func_name = func_name_tok.text;

        // 解析参数列表（()包裹，逻辑不变）
        std::vector<std::string> func_params;
        if (curr_token().type == TokenType::LParen) {
            skip_token("(");
            while (curr_token().type != TokenType::RParen) {
                const Token param_tok = skip_token();
                if (param_tok.type != TokenType::Identifier) {
                    std::cerr << Color::RED
                              << "[Syntax Error] Function parameter must be an identifier, got '"
                              << param_tok.text << "' (Line: " << param_tok.lineno << ")"
                              << Color::RESET << std::endl;
                    assert(false && "Invalid function parameter");
                }
                func_params.push_back(param_tok.text);
                // 处理参数间的逗号
                if (curr_token().type == TokenType::Comma) {
                    skip_token(",");
                } else if (curr_token().type != TokenType::RParen) {
                    std::cerr << Color::RED
                              << "[Syntax Error] Expected ',' or ')' in function parameters, got '"
                              << curr_token().text << "'"
                              << Color::RESET << std::endl;
                    assert(false && "Mismatched function parameters");
                }
            }
            skip_token(")");  // 跳过右括号
        }

        // 解析函数体（无大括号，用end结尾）
        skip_start_of_block();  // 跳过参数后的换行
        auto func_body = parse_block();  // 函数体为非全局作用域

        // 生成函数定义语句节点
        return std::make_unique<FuncDefStmt>(
            func_name,
            std::move(func_params),
            std::move(func_body)
        );
    }

    // 解析变量声明（var x = expr;）
    if (curr_tok.type == TokenType::Var) {
        skip_token("var");
        // 读取变量名（必须是标识符）
        const Token var_name_tok = skip_token();
        if (var_name_tok.type != TokenType::Identifier) {
            std::cerr << Color::RED
                      << "[Syntax Error] Variable name must be an identifier, got '"
                      << var_name_tok.text << "' (Line: " << var_name_tok.lineno << ")"
                      << Color::RESET << std::endl;
            assert(false && "Invalid variable name");
        }
        const std::string var_name = var_name_tok.text;

        // 解析赋值符号
        if (curr_token().type != TokenType::Assign) {
            std::cerr << Color::RED
                      << "[Syntax Error] Expected '=' in variable declaration, got '"
                      << curr_token().text << "'"
                      << Color::RESET << std::endl;
            assert(false && "Missing '=' in var declaration");
        }
        skip_token("=");

        // 解析赋值表达式
        auto init_expr = parse_expression();
        if (!init_expr) {
            std::cerr << Color::RED
                      << "[Syntax Error] Variable declaration missing initial value"
                      << Color::RESET << std::endl;
            assert(false && "Invalid var initial expression");
        }

        skip_end_of_ln();  // 跳过语句结束符
        return std::make_unique<VarDeclStmt>(var_name, std::move(init_expr));
    }

    // 解析return语句
    if (curr_tok.type == TokenType::Return) {
        skip_token("return");
        // return后可跟表达式（也可无，视为返回nil）
        std::unique_ptr<Expression> return_expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<ReturnStmt>(std::move(return_expr));
    }

    // 解析break语句
    if (curr_tok.type == TokenType::Break) {
        skip_token("break");
        skip_end_of_ln();
        return std::make_unique<BreakStmt>();
    }

    // 解析continue语句
    if (curr_tok.type == TokenType::Continue) {
        skip_token("continue");
        skip_end_of_ln();
        return std::make_unique<ContinueStmt>();
    }

    // 解析import语句
    if (curr_tok.type == TokenType::Import) {
        skip_token("import");
        // 读取模块路径（假设为字符串字面量，此处简化为标识符）
        const Token path_tok = skip_token();
        if (path_tok.type != TokenType::String && path_tok.type != TokenType::Identifier) {
            std::cerr << Color::RED
                      << "[Syntax Error] Import path must be a string or identifier, got '"
                      << path_tok.text << "' (Line: " << path_tok.lineno << ")"
                      << Color::RESET << std::endl;
            assert(false && "Invalid import path");
        }
        const std::string import_path = path_tok.text;

        skip_end_of_ln();
        return std::make_unique<ImportStmt>(import_path);
    }

    // 解析赋值语句（x = expr;）
    if (curr_tok.type == TokenType::Identifier && curr_tok_idx_ + 1 < tokens_.size()) {
        const Token next_tok = tokens_.at(curr_tok_idx_ + 1);
        if (next_tok.type == TokenType::Assign) {
            const std::string var_name = skip_token().text;  // 读取赋值目标变量名
            skip_token("=");                                 // 跳过赋值符号
            auto assign_expr = parse_expression();           // 解析赋值表达式
            if (!assign_expr) {
                std::cerr << Color::RED
                          << "[Syntax Error] Assignment missing right-hand side expression"
                          << Color::RESET << std::endl;
                assert(false && "Invalid assignment expression");
            }
            skip_end_of_ln();
            return std::make_unique<AssignStmt>(var_name, std::move(assign_expr));
        }
    }

    // 解析表达式语句（如函数调用、变量引用等）
    auto expr = parse_expression();
    if (expr != nullptr and curr_token().text == "=") {
        if (dynamic_cast<GetMemberExpr*>(expr.get())) {
            skip_token("=");
            auto value = parse_expression();
            skip_end_of_ln();

            auto set_mem = std::make_unique<SetMemberExpr>(std::move(expr), std::move(value));
            return std::make_unique<ExprStmt>(std::move(set_mem));
        }
        //非成员访问表达式后不能跟 =
        assert("invalid assignment target: expected member access");
    }
    if (expr != nullptr) {
        skip_end_of_ln();
        return std::make_unique<ExprStmt>(std::move(expr));
    }

    // 跳过无效Token（容错处理）
    while (curr_tok_idx_ < tokens_.size() && curr_token().type != TokenType::EndOfLine) {
        skip_token();  // 跳过当前无效Token
    }
    if (curr_tok_idx_ < tokens_.size()) {
        skip_token();  // 跳过换行符
    }
    return nullptr;  // 无有效语句，返回空
}

}
