#include "parser.hpp"

#include <algorithm>
#include <memory>
#include <vector>

#include "project_debugger.hpp"


namespace kiz {
std::unique_ptr<Expression> Parser::parse_expression() {
    DEBUG_OUTPUT("parse the expression...");
    return parse_logical_or();
}

// 处理 or
std::unique_ptr<Expression> Parser::parse_logical_or() {
    DEBUG_OUTPUT("parsing logical_or...");
    auto node = parse_logical_and(); // 先解析 and
    while (curr_token().type == TokenType::Or) {
        auto op_token = skip_token("or");
        auto right = parse_logical_and();
        node = std::make_unique<BinaryExpr>(
            std::move(op_token.text),
            std::move(node),
            std::move(right)
        );
    }
    return node;
}

// 处理 and
std::unique_ptr<Expression> Parser::parse_logical_and() {
    DEBUG_OUTPUT("parsing logical_and...");
    auto node = parse_comparison();
    while (curr_token().type == TokenType::And) {
        auto op_token = skip_token("and");
        auto right = parse_comparison();
        node = std::make_unique<BinaryExpr>(
            std::move(op_token.text),
            std::move(node),
            std::move(right)
        );
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_comparison() {
    DEBUG_OUTPUT("parsing comparison...");
    auto node = parse_add_sub();
    while (true) {
        const auto curr_type = curr_token().type;
        std::string op_text; // 存储运算符文本（如 "==" "in" "not in"）

        if (curr_type == TokenType::Equal ||
            curr_type == TokenType::NotEqual ||
            curr_type == TokenType::Greater ||
            curr_type == TokenType::Less ||
            curr_type == TokenType::GreaterEqual ||
            curr_type == TokenType::LessEqual) {
            auto op_token = skip_token();
            op_text = std::move(op_token.text);
        }
        // 处理in
        else if (curr_type == TokenType::In) {
            auto op_token = skip_token();
            op_text = "in";
        }
        // 处理not in'
        else if (
            curr_type == TokenType::Not
            and curr_tok_idx_ + 1 < tokens_.size()
            and tokens_[curr_tok_idx_ + 1].type == TokenType::In
        ) {
            skip_token("not");
            skip_token("in");
            op_text = "not in";
        }
        else {
            break;
        }
        auto right = parse_add_sub();
        node = std::make_unique<BinaryExpr>(
            std::move(op_text),
            std::move(node),
            std::move(right)
        );
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_add_sub() {
    DEBUG_OUTPUT("parsing add/sub...");
    auto node = parse_mul_div_mod();
    while (
        curr_token().type == TokenType::Plus
        or curr_token().type == TokenType::Minus
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_mul_div_mod();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_mul_div_mod() {
    DEBUG_OUTPUT("parsing mul/div/mod...");
    auto node = parse_power();
    while (
        curr_token().type == TokenType::Star
        or curr_token().type == TokenType::Slash
        or curr_token().type == TokenType::Percent
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_power() {
    DEBUG_OUTPUT("parsing power...");
    auto node = parse_unary();
    if (curr_token().type == TokenType::Caret) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();  // 右结合
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_unary() {
    DEBUG_OUTPUT("parsing unary...");
    if (curr_token().type == TokenType::Not) {
        auto op_token = skip_token(); // 跳过 not
        auto operand = parse_unary(); // 右结合
        return std::make_unique<UnaryExpr>(
            std::move(op_token.text),
            std::move(operand)
        );
    }
    if (curr_token().type == TokenType::Minus) {
        skip_token();
        auto operand = parse_unary();
        return std::make_unique<UnaryExpr>("-", std::move(operand));
    }
    return parse_factor();
}

std::unique_ptr<Expression> Parser::parse_factor() {
    DEBUG_OUTPUT("parsing factor...");
    auto node = parse_primary();

    while (true) {
        if (curr_token().type == TokenType::Dot) {
            skip_token(".");
            auto child = std::make_unique<IdentifierExpr>(skip_token().text);
            node = std::make_unique<GetMemberExpr>(std::move(node),std::move(child));

        }
        else if (curr_token().type == TokenType::LBracket) {
            skip_token("[");
            auto param = parse_params(TokenType::RBracket);
            skip_token("]");
            node = std::make_unique<GetItemExpr>(std::move(node),std::move(param));
        }
        else if (curr_token().type == TokenType::LParen) {
            skip_token("(");
            auto param = parse_params(TokenType::RParen);
            skip_token(")");
            node = std::make_unique<CallExpr>(std::move(node),std::move(param));
        }
        else break;
    }
    return node;
}

std::unique_ptr<Expression> Parser::parse_primary() {
    DEBUG_OUTPUT("parsing primary...");
    const auto tok = skip_token();
    if (tok.type == TokenType::Number) {
        return std::make_unique<NumberExpr>(tok.text);
    }
    if (tok.type == TokenType::String) {
        return std::make_unique<StringExpr>(tok.text);
    }
    if (tok.type == TokenType::Null) {
        return std::make_unique<StringExpr>(tok.text);
    }
    if (tok.type == TokenType::True) {
        return std::make_unique<StringExpr>(tok.text);
    }
    if (tok.type == TokenType::False) {
        return std::make_unique<StringExpr>(tok.text);
    }
    if (tok.type == TokenType::Identifier) {
        return std::make_unique<IdentifierExpr>(tok.text);
    }
    if (tok.type == TokenType::Func) {
        std::vector<std::string> params{};
        if (curr_token().type == TokenType::Pipe) {
            skip_token("|");
            while (curr_token().type != TokenType::Pipe) {
                params.emplace_back(skip_token().text);
                if (curr_token().type == TokenType::Comma) skip_token(",");
            }
            skip_token("|");
        }

        skip_token("{");
        auto stmt = parse_block();
        skip_token("}");
        return std::make_unique<LambdaDeclExpr>("<lambda>", std::move(params),std::move(stmt));
    }
    if (tok.type == TokenType::Pipe) {
        std::vector<std::string> params;
        while (curr_token().type != TokenType::Pipe) {
            params.emplace_back(skip_token().text);
            if (curr_token().type == TokenType::Comma) skip_token(",");
        }
        skip_token("|");
        auto expr = parse_expression();
        std::vector<std::unique_ptr<Statement>> stmts;
        stmts.emplace_back(std::make_unique<ReturnStmt>(std::move(expr)));

        return std::make_unique<LambdaDeclExpr>(
            "lambda",
            std::move(params),
            std::make_unique<BlockStmt>(std::move(stmts))
        );
    }
    if (tok.type == TokenType::LBrace) {
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> init_vec{};
        while (curr_token().type != TokenType::RBrace) {
            auto key = skip_token().text;
            skip_token("=");
            auto val = parse_expression();
            if (curr_token().type == TokenType::Comma) skip_token(",");
            if (curr_token().type == TokenType::Semicolon) skip_token(";");
            init_vec.emplace_back(std::move(key), std::move(val));
        }
        skip_token("}");
        return std::make_unique<DictDeclExpr>("<lambda_dict>", std::move(init_vec));
    }
    if (tok.type == TokenType::LBracket) {
        auto param = parse_params(TokenType::RBracket);
        skip_token("]");
        return std::make_unique<ListExpr>(std::move(param));
    }
    if (tok.type == TokenType::LParen) {
        auto expr = parse_expression();
        skip_token(")");
        return expr;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Expression>> Parser::parse_params(const TokenType endswith){
    std::vector<std::unique_ptr<Expression>> params;
    while (curr_token().type != endswith) {
        params.emplace_back(parse_expression());
        if (curr_token().type == TokenType::Comma) skip_token(",");
    }
    return params;
}

}
