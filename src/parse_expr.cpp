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
    auto node = parse_a_token();

    while (true) {
        if (curr_token().type == kiz::TokenType::ExclamationMark) {
            skip_token("!");
            node = std::make_unique<UnaryExpr>("!", std::move(node));
        }
        if (curr_token().type == kiz::TokenType::Dot) {
            node = parse_get_member(std::move(node));
        }
        else if (curr_token().type == kiz::TokenType::DoubleColon) {
            node = parse_namespace_get_member(std::move(node));
        }
        else if (curr_token().type == kiz::TokenType::LBracket) {
            node = parse_get_item(std::move(node));
        }
        else if (curr_token().type == kiz::TokenType::LParen) {
            node = parse_func_call(std::move(node));
        }
        else {
            break;
        }
    }
    return node;
}
