/**
 * @file parser.hpp
 * @brief 语法分析器（Parser）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

namespace kiz {

class Parser {
    const std::vector<Token> tokens_;
    long long unsigned int curr_tok_idx_ = 0;
public:
    explicit Parser(const std::vector<Token>& tokens);
    ~Parser() = default;

    Token skip_token(const std::string& want_skip = "");
    void skip_end_of_ln();
    void skip_start_of_block();
    [[nodiscard]] Token curr_token() const;

    std::vector<std::unique_ptr<Statement>> parse();

    // parse stmt
    std::unique_ptr<Statement> parse_stmt();
    std::unique_ptr<BlockStmt> parse_block();

    // parse expr
    std::unique_ptr<Expression> parse_expression();
    std::unique_ptr<Expression> parse_logical_and();
    std::unique_ptr<Expression> parse_logical_or();
    std::unique_ptr<Expression> parse_comparison();
    std::unique_ptr<Expression> parse_add_sub();
    std::unique_ptr<Expression> parse_mul_div_mod();
    std::unique_ptr<Expression> parse_power();
    std::unique_ptr<Expression> parse_unary();
    std::unique_ptr<Expression> parse_factor();

    // parse factor
    std::unique_ptr<Expression> parse_primary();
    std::vector<std::unique_ptr<Expression>> parse_params(TokenType endswith);
};

} // namespace kiz