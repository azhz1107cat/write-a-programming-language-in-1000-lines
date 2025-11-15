/**
 * @file parser.cpp
 * @brief 语法分析器（Parser）核心实现
 * 从Token列表生成AST（适配函数定义新语法：fn x() end）
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "parser.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <iostream>
#include <memory>
#include <vector>

#include "ui/color.hpp"

namespace kiz {

Parser::Parser(const std::vector<Token>& tokens) {
    tokens_ = tokens;    // 绑定Token列表
    curr_tok_idx_ = 0;    // 初始化索引为0（从第一个Token开始）
}

Token Parser::skip_token(const std::string& want_skip) {
    if (curr_tok_idx_ < tokens_.size()) {
        const Token& curr_tok = tokens_.at(curr_tok_idx_);
        // 若指定了期望文本，校验当前Token是否匹配
        if (!want_skip.empty() && curr_tok.text != want_skip) {
            // std::cerr << ConClr::RED
            //           << "[Syntax Error] Expected '" << want_skip << "', but got '"
            //           << curr_tok.text << "' (Line: " << curr_tok.line << ", Col: " << curr_tok.col << ")"
            //           << ConClr::RESET << std::endl;
            assert(false && "Syntax mismatch in skip_token");
        }
        curr_tok_idx_++;  // 移动到下一个Token
        return curr_tok;
    }
    // 到达Token列表末尾，返回EOF标记
    return {TokenType::EndOfFile, "", 0, 0};
}

// curr_token实现
Token Parser::curr_token() const {
    if (curr_tok_idx_ < tokens_.size()) {
        return tokens_.at(curr_tok_idx_);
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
              << curr_tok.text << "' (Line: " << curr_tok.lineno << ", Col: " << curr_tok.column << ")"
              << ConClr::RESET << std::endl;
    assert(false && "Invalid statement terminator");
}

// skip_start_of_block实现 处理函数体前置换行
void Parser::skip_start_of_block() {
    // 跳过函数定义后所有连续的换行符，确保正确解析函数体第一条语句
    while (curr_tok_idx_ < tokens_.size()) {
        const Token& curr_tok = tokens_.at(curr_tok_idx_);
        if (curr_tok.type == TokenType::EndOfLine) {
            skip_token("\n");
        } else {
            break;  // 遇到非换行Token，停止跳过
        }
    }
}

// parse_program实现（解析整个程序
std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> program_stmts;
    // 循环解析所有语句，直到EOF
    while (curr_token().type != TokenType::EndOfFile) {
        if (auto stmt = parse_stmt(); stmt != nullptr) {
            program_stmts.push_back(std::move(stmt));
        }
    }
    return program_stmts;
}

} // namespace kiz