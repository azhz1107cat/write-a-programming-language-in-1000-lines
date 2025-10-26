/**
 * @file parse_expr.cpp
 * @brief 语法分析 解释表达式部分 的核心实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

std::unique_ptr<Expression> kiz::Parser::parse_expression() {
    // ToDo: add op support 'and' 'or' 'not in' 'in'
    return parse_comparison();
}

std::unique_ptr<Expression> kiz::Parser::parse_comparison() {
    auto node = parse_add_sub();
    while (
        curr_token().type == kiz::TokenType::Equal
        or curr_token().type == kiz::TokenType::NotEqual
        or curr_token().type == kiz::TokenType::Greater
        or curr_token().type == kiz::TokenType::Less
        or curr_token().type == kiz::TokenType::GreaterEqual
        or curr_token().type == kiz::TokenType::LessEqual
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_add_sub();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> kiz::Parser::parse_add_sub() {
    auto node = parse_mul_div_mod();
    while (
        curr_token().type == kiz::TokenType::Plus
        or curr_token().type == kiz::TokenType::Minus
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_mul_div_mod();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> kiz::Parser::parse_mul_div_mod() {
    auto node = parse_power();
    while (
        curr_token().type == kiz::TokenType::Star
        or curr_token().type == kiz::TokenType::Slash
        or curr_token().type == kiz::TokenType::Percent
    ) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> kiz::Parser::parse_power() {
    auto node = parse_unary();
    if (curr_token().type == kiz::TokenType::Caret) {
        auto tok = curr_token();
        auto op = skip_token().text;
        auto right = parse_power();  // 右结合
        node = std::make_unique<BinaryExpr>(std::move(op), std::move(node), std::move(right));
    }
    return node;
}

std::unique_ptr<Expression> kiz::Parser::parse_unary() {
    // ToDo: add op support 'not'
    if (curr_token().type == kiz::TokenType::Minus) {
        auto tok = curr_token();
        skip_token();
        auto operand = parse_unary();
        return std::make_unique<UnaryExpr>("-", std::move(operand));
    }
    return parse_factor();
}

std::unique_ptr<Expression> kiz::Parser::parse_factor() {
    auto node = parse_primary();

    while (true) {
        if (curr_token().type == kiz::TokenType::Dot) {
            skip_token(".");
            auto child = std::make_unique<IdentifierExpr>(skip_token().text);
            node = std::make_unique<GetMemberExpr>(std::move(node),std::move(child));

        }
        else if (curr_token().type == kiz::TokenType::LBracket) {
            skip_token("[");
            auto param = parse_params(kiz::TokenType::RBracket);
            skip_token("]");
            node = std::make_unique<GetItemExpr>(std::move(node),std::move(param));
        }
        else if (curr_token().type == kiz::TokenType::LParen) {
            skip_token("(");
            auto param = parse_params(kiz::TokenType::RParen);
            skip_token(")");
            node = std::make_unique<CallExpr>(std::move(node),std::move(param));
        }
        else break;
    }
    return node;
}

std::unique_ptr<Expression> kiz::Parser::parse_primary() {
    const auto tok = skip_token();
    if (tok.type == kiz::TokenType::Number) {
        return std::make_unique<LiteralExpr>(tok.text);
    }
    if (tok.type == kiz::TokenType::String) {
        return std::make_unique<LiteralExpr>(tok.text);
    }
    if (tok.type == kiz::TokenType::Null) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Null);
    }
    if (tok.type == kiz::TokenType::True) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Bool);
    }
    if (tok.type == kiz::TokenType::False) {
        return std::make_unique<LiteralExpr>(tok.text, Value::Type::Bool);
    }
    if (tok.type == kiz::TokenType::Identifier) {
        return std::make_unique<IdentifierExpr>(tok.text);
    }
    if (tok.type == kiz::TokenType::Lambda) {
        std::vector<std::string> params{};
        if (curr_token().type == kiz::TokenType::Pipe) {
            skip_token("|");
            while (curr_token().type != kiz::TokenType::Pipe) {
                params.emplace_back(skip_token().text);
                if (curr_token().type == kiz::TokenType::Comma) skip_token(",");
            }
            skip_token("|");
        }

        skip_token("{");
        auto stmt = parse_block(true);
        skip_token("}");
        return std::make_unique<LambdaDeclExpr>("<lambda>", std::move(params),std::move(stmt));
    }
    if (tok.type == kiz::TokenType::Pipe) {
        std::vector<std::string> params;
        while (curr_token().type != kiz::TokenType::Pipe) {
            params.emplace_back(skip_token().text);
            if (curr_token().type == kiz::TokenType::Comma) skip_token(",");
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
    if (tok.type == kiz::TokenType::LBrace) {
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> init_vec{};
        while (curr_token().type != kiz::TokenType::RBrace) {
            auto key = skip_token().text;
            skip_token("=");
            auto val = parse_expression();
            if (curr_token().type == kiz::TokenType::Comma) skip_token(",");
            if (curr_token().type == kiz::TokenType::Semicolon) skip_token(";");
            init_vec.emplace_back(std::move(key), std::move(val));
        }
        skip_token("}");
        return std::make_unique<LambdaStructDeclExpr>(std::move(init_vec));
    }
    if (tok.type == kiz::TokenType::LBracket) {
        auto param = parse_params(kiz::TokenType::RBracket);
        skip_token("]");
        return std::make_unique<ArrayExpr>(std::move(param));
    }
    if (tok.type == kiz::TokenType::LParen) {
        auto expr = parse_expression();
        skip_token(")");
        return expr;
    }
    return nullptr;
}

std::vector<std::unique_ptr<Expression>> kiz::Parser::parse_params(const kiz::TokenType endswith){
    std::vector<std::unique_ptr<Expression>> params;
    while (curr_token().type != endswith) {
        params.emplace_back(parse_expression());
        if (curr_token().type == kiz::TokenType::Comma) skip_token(",");
    }
    return params;
}
