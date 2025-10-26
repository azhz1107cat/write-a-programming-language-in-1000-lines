/**
 * @file parser.cpp
 * @brief 语法分析器（Parser）核心实现
 * 从Token列表生成AST
 * @author azhz1107cat
 * @date 2025-10-25
 */

kiz::Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens){};

Token kiz::Parser::skip_token(const std::string& want_skip) {
    if (curr_tok_idx_ < tokens_.size()) {
        auto& tok = tokens_[curr_tok_idx_];
        if (!want_skip.empty() and tok.text != want_skip) {
            std::cerr << ConClr::RED
            << "There should be '" << want_skip << "' , but you given '"
            << tok.text << "'" << ConClr::RESET << std::endl;
            throw StdLibException("");
        }
        curr_tok_idx_++;
        return tok;
    }
    return {kiz::TokenType::EndOfFile, "", 0, 0};
}

Token kiz::Parser::curr_token() const {
    if (curr_tok_idx_ < tokens_.size()) {
        auto& tok = tokens_[curr_tok_idx_];
        return tok;
    }
    return {kiz::TokenType::EndOfFile, "", 0, 0};
}

void kiz::Parser::skip_end_of_ln() {
    const Token tok = curr_token();
    if (tok.type == kiz::TokenType::Semicolon) {
        skip_token(";");
        return;
    }
    if (tok.type == kiz::TokenType::EndOfLine) {
        skip_token("\n");
        return;
    }
    if (tok.type == kiz::TokenType::EndOfFile) {
        return;
    }
    std::cerr << ConClr::RED << "End of line must be ';', got '" << tok.text << "'" << ConClr::RESET << std::endl;
    throw StdLibException("");
}

void kiz::Parser::skip_start_of_block() {
}

std::vector<std::unique_ptr<Statement>> kiz::Parser::parse_program() {
    std::vector<std::unique_ptr<Statement>>
        stmts = {};
    while (curr_token().type != kiz::TokenType::EndOfFile) {
        if (auto s = parse_stmt();
            s != nullptr
        ) {
            stmts.push_back(std::move(s));
        }
    }
    return stmts;
}

std::unique_ptr<BlockStmt> kiz::Parser::parse_block(bool is_global) {
    std::vector<std::unique_ptr<Statement>> stmts;
    while (curr_token().type != kiz::TokenType::RBrace) {
        stmts.emplace_back(parse_stmt());
        if (curr_token().type == kiz::TokenType::Semicolon) skip_token(";");
    }
    return std::make_unique<BlockStmt>(std::move(stmts));
}

std::unique_ptr<Statement> Parser::parse_stmt() {
    auto tok = curr_token();

    if (tok.type == kiz::TokenType::If) {
        skip_token("if");
        auto expr = parse_expression();
        skip_token("{");
        auto stmts = parse_block(true);
        skip_token("}");
        std::unique_ptr<BlockStmt> else_stmts = nullptr;
        if (curr_token().type == kiz::TokenType::Else) {
            skip_token("else");
            if (curr_token().type == kiz::TokenType::If) {
                skip_token("if");
                std::vector<std::unique_ptr<Statement>> if_branch = {};
                if_branch.emplace_back(std::move(parse_if()));
                else_stmts = std::make_unique<BlockStmt>(std::move(if_branch));
            } else {
                skip_token("{");
                else_stmts = parse_block(true);
                skip_token("}");
            }
        }
        return std::make_unique<IfStmt>(std::move(expr),std::move(stmts),std::move(else_stmts));
    }

    if (tok.type == kiz::TokenType::While) {
        skip_token("while");
        auto expr = parse_expression();
        skip_token("{");
        auto stmts = parse_block(true);
        skip_token("}");
        return std::make_unique<WhileStmt>(std::move(expr), std::move(stmts));
    }

    if (tok.type == kiz::TokenType::Func) {
        skip_token("func");
        auto name = skip_token().text;
        std::vector<std::string> params;

        if (curr_token().type == kiz::TokenType::LParen) {
            skip_token("(");
            while (curr_token().type != kiz::TokenType::RParen) {
                params.emplace_back(skip_token().text);
                if (curr_token().type == kiz::TokenType::Comma) skip_token(",");
            }
            skip_token(")");
        }

        skip_token("{");
        auto stmts = parse_block(true);
        skip_token("}");
        return std::make_unique<FuncDefStmt>(name, std::move(params), std::move(stmts));
    }

    if (tok.type == kiz::TokenType::Var) {
        skip_token("var");
        auto name = skip_token().text;

        skip_token("=");
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<VarDeclStmt>(name, std::move(expr));
    }

    if (tok.type == kiz::TokenType::Return) {
        skip_token("return");
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<ReturnStmt>(std::move(expr));
    }

    if (tok.type == kiz::TokenType::Break) {
        skip_token("break");
        skip_end_of_ln();
        return std::make_unique<BreakStmt>();
    }

    if (tok.type == kiz::TokenType::Continue) {
        skip_token("continue");
        skip_end_of_ln();
        return std::make_unique<ContinueStmt>();
    }

    if (tok.type == kiz::TokenType::Import) {
        skip_token("import");
        const auto path = skip_token().text;
        skip_end_of_ln();
        return std::make_unique<ImportStmt>(path);
    }

    if (tok.type == kiz::TokenType::Identifier
        and curr_tok_idx_ + 1 < tokens_.size()
        and tokens_[curr_tok_idx_ + 1].type == kiz::TokenType::Assign
    ) {
        const auto name = skip_token().text;
        skip_token("=");
        auto expr = parse_expression();
        skip_end_of_ln();
        return std::make_unique<AssignStmt>(name, std::move(expr));
    }

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
        throw StdLibException("invalid assignment target: expected member access");
    }

    if (expr != nullptr) {
        skip_end_of_ln();
        return std::make_unique<ExprStmt>(std::move(expr));
    }

    while (curr_tok_idx_ < tokens_.size() && curr_token().type != kiz::TokenType::EndOfLine) {
        skip_token(); 
    }
    if (curr_tok_idx_ < tokens_.size()) {
        skip_token(); // 跳过 EndOfLine
    }
    return nullptr;
}