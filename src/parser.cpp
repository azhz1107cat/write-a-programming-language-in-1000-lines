/**
 * @file parser.cpp
 * @brief 语法分析器（Parser）核心实现
 * 从Token列表生成AST（适配函数定义新语法：fn x() end）
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include <vector>
#include <memory>
#include <cerrno>
#include <cassert>
#include <algorithm>
#include <iostream>

namespace kiz {

Parser::Parser(const std::vector<Token>& tokens) {
    Parser::tokens_ = &tokens;    // 绑定Token列表
    Parser::curr_tok_idx_ = 0;    // 初始化索引为0（从第一个Token开始）
}

Token Parser::skip_token() {
    return skip_token("");  // 调用带参数版本，空字符串表示不校验文本
}


Token Parser::skip_token(const std::string& want_skip) {
    if (curr_tok_idx_ < tokens_->size()) {
        const Token& curr_tok = tokens_->at(curr_tok_idx_);
        // 若指定了期望文本，校验当前Token是否匹配
        if (!want_skip.empty() && curr_tok.text != want_skip) {
            std::cerr << ConClr::RED
                      << "[Syntax Error] Expected '" << want_skip << "', but got '"
                      << curr_tok.text << "' (Line: " << curr_tok.line << ", Col: " << curr_tok.col << ")"
                      << ConClr::RESET << std::endl;
            std::assert(false && "Syntax mismatch in skip_token");
        }
        curr_tok_idx_++;  // 移动到下一个Token
        return curr_tok;
    }
    // 到达Token列表末尾，返回EOF标记
    return {TokenType::EndOfFile, "", 0, 0};
}

// curr_token实现
Token Parser::curr_token() const {
    if (curr_tok_idx_ < tokens_->size()) {
        return tokens_->at(curr_tok_idx_);
    }
    return {TokenType::EndOfFile, "", 0, 0};
}

// skip_end_of_ln实现
void Parser::skip_end_of_ln() {
    const Token curr_tok = curr_token();
    // 支持分号或换行作为语句结束符
    if (curr_tok.type == TokenType::Semicolon) {
        skip_token(";");
        return;
    }
    if (curr_tok.type == TokenType::EndOfLine) {
        skip_token("\n");
        return;
    }
    // 到达文件末尾也视为合法结束
    if (curr_tok.type == TokenType::EndOfFile) {
        return;
    }
    // 既不是分号也不是换行，抛错
    std::cerr << ConClr::RED
              << "[Syntax Error] Statement must end with ';' or newline, got '"
              << curr_tok.text << "' (Line: " << curr_tok.line << ", Col: " << curr_tok.col << ")"
              << ConClr::RESET << std::endl;
    std::assert(false && "Invalid statement terminator");
}

// skip_start_of_block实现 处理函数体前置换行
void Parser::skip_start_of_block() {
    // 跳过函数定义后所有连续的换行符，确保正确解析函数体第一条语句
    while (curr_tok_idx_ < tokens_->size()) {
        const Token& curr_tok = tokens_->at(curr_tok_idx_);
        if (curr_tok.type == TokenType::EndOfLine) {
            skip_token("\n");
        } else {
            break;  // 遇到非换行Token，停止跳过
        }
    }
}

// parse_program实现（解析整个程序
std::vector<std::unique_ptr<Statement>> Parser::parse_program() {
    std::vector<std::unique_ptr<Statement>> program_stmts;
    // 循环解析所有语句，直到EOF
    while (curr_token().type != TokenType::EndOfFile) {
        if (auto stmt = parse_stmt(); stmt != nullptr) {
            program_stmts.push_back(std::move(stmt));
        }
    }
    return program_stmts;
}

// parse_block
std::unique_ptr<BlockStmt> Parser::parse_block(bool is_global) {
    std::vector<std::unique_ptr<Statement>> block_stmts;
    // 循环解析块内语句，直到遇到end关键字（替代原RBrace）
    while (curr_tok_idx_ < tokens_->size()) {
        const Token& curr_tok = curr_token();
        if (curr_tok.type == TokenType::End) {
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
    if (end_tok.type != TokenType::End) {
        std::cerr << ConClr::RED
                  << "[Syntax Error] Block must end with 'end', got '"
                  << end_tok.text << "' (Line: " << end_tok.line << ", Col: " << end_tok.col << ")"
                  << ConClr::RESET << std::endl;
        std::assert(false && "Block missing 'end' terminator");
    }
    // 跳过end关键字，完成块解析
    skip_token("end");
    return std::make_unique<BlockStmt>(std::move(block_stmts), is_global);
}

// parse_if实现
std::unique_ptr<IfStmt> Parser::parse_if() {
    // 解析if条件表达式
    auto cond_expr = parse_expression();
    if (!cond_expr) {
        std::cerr << ConClr::RED
                  << "[Syntax Error] If statement missing condition expression"
                  << ConClr::RESET << std::endl;
        std::assert(false && "Invalid if condition");
    }
    // 解析if体（无大括号，直接调用parse_block）
    skip_start_of_block();  // 跳过条件后的换行
    auto if_block = parse_block(false);
    // 处理else分支（默认空）
    std::unique_ptr<BlockStmt> else_block = nullptr;
    if (curr_token().type == TokenType::Else) {
        skip_token("else");
        if (curr_token().type == TokenType::If) {
            // else if分支：递归解析if语句，包装为BlockStmt
            std::vector<std::unique_ptr<Statement>> else_if_stmts;
            else_if_stmts.push_back(parse_if());
            else_block = std::make_unique<BlockStmt>(std::move(else_if_stmts), false);
        } else {
            // else分支：直接解析块
            skip_start_of_block();
            else_block = parse_block(false);
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
            std::cerr << ConClr::RED
                      << "[Syntax Error] While statement missing condition expression"
                      << ConClr::RESET << std::endl;
            std::assert(false && "Invalid while condition");
        }
        // 解析循环体（无大括号，用end结尾）
        skip_start_of_block();
        auto while_block = parse_block(false);
        return std::make_unique<WhileStmt>(std::move(cond_expr), std::move(while_block));
    }

    // 解析函数定义（新语法：fn x() end）
    if (curr_tok.type == TokenType::Func) {
        skip_token("func");
        // 读取函数名（必须是标识符）
        const Token func_name_tok = skip_token();
        if (func_name_tok.type != TokenType::Identifier) {
            std::cerr << ConClr::RED
                      << "[Syntax Error] Function name must be an identifier, got '"
                      << func_name_tok.text << "' (Line: " << func_name_tok.line << ")"
                      << ConClr::RESET << std::endl;
            std::assert(false && "Invalid function name");
        }
        const std::string func_name = func_name_tok.text;

        // 解析参数列表（()包裹，逻辑不变）
        std::vector<std::string> func_params;
        if (curr_token().type == TokenType::LParen) {
            skip_token("(");
            while (curr_token().type != TokenType::RParen) {
                const Token param_tok = skip_token();
                if (param_tok.type != TokenType::Identifier) {
                    std::cerr << ConClr::RED
                              << "[Syntax Error] Function parameter must be an identifier, got '"
                              << param_tok.text << "' (Line: " << param_tok.line << ")"
                              << ConClr::RESET << std::endl;
                    std::assert(false && "Invalid function parameter");
                }
                func_params.push_back(param_tok.text);
                // 处理参数间的逗号
                if (curr_token().type == TokenType::Comma) {
                    skip_token(",");
                } else if (curr_token().type != TokenType::RParen) {
                    std::cerr << ConClr::RED
                              << "[Syntax Error] Expected ',' or ')' in function parameters, got '"
                              << curr_token().text << "'"
                              << ConClr::RESET << std::endl;
                    std::assert(false && "Mismatched function parameters");
                }
            }
            skip_token(")");  // 跳过右括号
        }

        // 解析函数体（无大括号，用end结尾）
        skip_start_of_block();  // 跳过参数后的换行
        auto func_body = parse_block(false);  // 函数体为非全局作用域

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
            std::cerr << ConClr::RED
                      << "[Syntax Error] Variable name must be an identifier, got '"
                      << var_name_tok.text << "' (Line: " << var_name_tok.line << ")"
                      << ConClr::RESET << std::endl;
            std::assert(false && "Invalid variable name");
        }
        const std::string var_name = var_name_tok.text;

        // 解析赋值符号
        if (curr_token().type != TokenType::Assign) {
            std::cerr << ConClr::RED
                      << "[Syntax Error] Expected '=' in variable declaration, got '"
                      << curr_token().text << "'"
                      << ConClr::RESET << std::endl;
            std::assert(false && "Missing '=' in var declaration");
        }
        skip_token("=");

        // 解析赋值表达式
        auto init_expr = parse_expression();
        if (!init_expr) {
            std::cerr << ConClr::RED
                      << "[Syntax Error] Variable declaration missing initial value"
                      << ConClr::RESET << std::endl;
            std::assert(false && "Invalid var initial expression");
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
        if (path_tok.type != TokenType::StringLiteral && path_tok.type != TokenType::Identifier) {
            std::cerr << ConClr::RED
                      << "[Syntax Error] Import path must be a string or identifier, got '"
                      << path_tok.text << "' (Line: " << path_tok.line << ")"
                      << ConClr::RESET << std::endl;
            std::assert(false && "Invalid import path");
        }
        const std::string import_path = path_tok.text;

        skip_end_of_ln();
        return std::make_unique<ImportStmt>(import_path);
    }

    // 解析赋值语句（x = expr;）
    if (curr_tok.type == TokenType::Identifier && curr_tok_idx_ + 1 < tokens_->size()) {
        const Token next_tok = tokens_->at(curr_tok_idx_ + 1);
        if (next_tok.type == TokenType::Assign) {
            const std::string var_name = skip_token().text;  // 读取赋值目标变量名
            skip_token("=");                                 // 跳过赋值符号
            auto assign_expr = parse_expression();           // 解析赋值表达式
            if (!assign_expr) {
                std::cerr << ConClr::RED
                          << "[Syntax Error] Assignment missing right-hand side expression"
                          << ConClr::RESET << std::endl;
                std::assert(false && "Invalid assignment expression");
            }
            skip_end_of_ln();
            return std::make_unique<AssignStmt>(var_name, std::move(assign_expr));
        }
    }

    // 解析表达式语句（如函数调用、变量引用等）
    auto expr_stmt = parse_expression();
    if (expr_stmt != nullptr) {
        // 处理成员赋值（如 obj.prop = val;）
        if (curr_token().type == TokenType::Assign) {
            if (auto get_mem_expr = dynamic_cast<GetMemberExpr*>(expr_stmt.get())) {
                // 拷贝成员访问表达式（避免unique_ptr所有权问题）
                auto cloned_get_mem = std::make_unique<GetMemberExpr>(
                    std::move(get_mem_expr->obj_expr),
                    get_mem_expr->prop_name
                );
                skip_token("=");
                auto val_expr = parse_expression();
                if (!val_expr) {
                    std::cerr << ConClr::RED
                              << "[Syntax Error] Member assignment missing value expression"
                              << ConClr::RESET << std::endl;
                    std::assert(false && "Invalid member assignment");
                }
                skip_end_of_ln();
                auto set_mem_expr = std::make_unique<SetMemberExpr>(
                    std::move(cloned_get_mem),
                    std::move(val_expr)
                );
                return std::make_unique<ExprStmt>(std::move(set_mem_expr));
            } else {
                // 非成员访问表达式不允许赋值（如 123 = 456;）
                std::cerr << ConClr::RED
                          << "[Syntax Error] Invalid assignment target: not a member access or variable"
                          << ConClr::RESET << std::endl;
                std::assert(false && "Invalid assignment target");
            }
        }
        // 普通表达式语句（如 add(1,2);）
        skip_end_of_ln();
        return std::make_unique<ExprStmt>(std::move(expr_stmt));
    }

    // 跳过无效Token（容错处理）
    while (curr_tok_idx_ < tokens_->size() && curr_token().type != TokenType::EndOfLine) {
        skip_token();  // 跳过当前无效Token
    }
    if (curr_tok_idx_ < tokens_->size()) {
        skip_token();  // 跳过换行符
    }
    return nullptr;  // 无有效语句，返回空
}

} // namespace kiz