/**
 * @file lexer.cpp
 * @brief 词法分析器（Lexer）核心实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

static std::map<std::string, kiz::TokenType> keywords;
static bool keywords_registered = false;

void registerKeywords() {
    if (keywords_registered) return;
    keywords["var"] = kiz::TokenType::Var;
    keywords["func"] = kiz::TokenType::Func;
    keywords["if"] = kiz::TokenType::If;
    keywords["else"] = kiz::TokenType::Else;
    keywords["while"] = kiz::TokenType::While;
    keywords["return"] = kiz::TokenType::Return;
    keywords["import"] = kiz::TokenType::Import;
    keywords["break"] = kiz::TokenType::Break;
    keywords["continue"] = kiz::TokenType::Continue;
    keywords["true"] = kiz::TokenType::True;
    keywords["false"] = kiz::TokenType::False;
    keywords["null"] = kiz::TokenType::Null;
    keywords_registered = true;
}

std::vector<Token> Lexer::tokenize(const std::string& src) {
    std::vector<Token> tokens;
    size_t i = 0;
    int line = 1, col = 1;
    registerKeywords();
    while (i < src.size()) {
        if (src[i] == '\n') {
            if ( !tokens.empty() ) {
                if (tokens.back().type == kiz::TokenType::Backslash) {
                    tokens.pop_back();
                }
                tokens.emplace_back(kiz::TokenType::EndOfLine, "\n", line, col);
            }
            ++line;
            col = 1;
            ++i;
            continue;
        }
        if (isspace(src[i])) {
            ++col;
            ++i;
            continue;
        }
        size_t start_col = col;

        if (isalpha(src[i]) || src[i] == '_') {
            size_t j = i + 1;
            while (j < src.size() && (isalnum(src[j]) || src[j] == '_')) ++j;
            std::string ident = src.substr(i, j - i);
            auto type = kiz::TokenType::Identifier;
            if (keywords.contains(ident)) type = keywords[ident];
            tokens.emplace_back(type, ident, line, start_col);
            col += (j - i);
            i = j;
        } else if (src[i] == '=' && i + 1 < src.size() && src[i + 1] == '>') {
            tokens.emplace_back(kiz::TokenType::FatArrow, "=>", line, start_col);
            i += 2;
            col += 2;
        }
        else if (src[i] == '-' && i + 1 < src.size() && src[i + 1] == '>') {
            tokens.emplace_back(kiz::TokenType::ThinArrow, "->", line, start_col);
            i += 2;
            col += 2;
        } else if (src[i] == '=' && i + 1 < src.size() && src[i + 1] == '=') {
            tokens.emplace_back(kiz::TokenType::Equal, "==", line, start_col);
            i += 2;
            col += 2;
        } else if (src[i] == '!' && i + 1 < src.size() && src[i + 1] == '=') {
            tokens.emplace_back(kiz::TokenType::NotEqual, "!=", line, start_col);
            i += 2;
            col += 2;
        } else if (src[i] == '!') {
            tokens.emplace_back(kiz::TokenType::ExclamationMark, "!", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '<' && i + 1 < src.size() && src[i + 1] == '=') {
            tokens.emplace_back(kiz::TokenType::LessEqual, "<=", line, start_col);
            i += 2;
            col += 2;
        } else if (src[i] == '>' && i + 1 < src.size() && src[i + 1] == '=') {
            tokens.emplace_back(kiz::TokenType::GreaterEqual, ">=", line, start_col);
            i += 2;
            col += 2;
        } else if (src[i] == '<') {
            tokens.emplace_back(kiz::TokenType::Less, "<", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '>') {
            tokens.emplace_back(kiz::TokenType::Greater, ">", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == ':') {
            ++i;
            ++col;
            if (src[i] == ':') {
                ++i;
                ++col;
                tokens.emplace_back(kiz::TokenType::DoubleColon, "::", line, start_col);
            }
        } else if (src[i] == '=') {
            tokens.emplace_back(kiz::TokenType::Assign, "=", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '>') {
            tokens.emplace_back(kiz::TokenType::Greater, ">", line, start_col);
            ++i;
            ++col;
        } else if (isdigit(src[i]) || (src[i] == '.' && i + 1 < src.size() && isdigit(src[i + 1]))) {
            size_t j = i;
            bool has_dot = false;
            bool has_underscore;

            // Handle decimal numbers including scientific notation
            // Parse mantissa (before 'e' or 'E')
            while (j < src.size()) {
                // 允许数字、单个小数点（仅一次）和下划线（作为分隔符）
                if (isdigit(src[j])) {
                    has_underscore = false; // 重置下划线标志
                    ++j;
                } else if (src[j] == '.' && !has_dot) {
                    has_dot = true;
                    has_underscore = false; // 重置下划线标志
                    ++j;
                } else if (src[j] == '_' && !has_underscore && j > i && j + 1 < src.size() && isdigit(src[j + 1])) {
                    has_underscore = true;
                    ++j;
                } else {
                    break;
                }
            }

            // 检查科学计数法（e或E）
            if (j < src.size() && (src[j] == 'e' || src[j] == 'E')) {
                size_t e_pos = j;
                ++j;    // 跳过'e'或'E'

                // 检查指数部分的可选正负号
                if (j < src.size() && (src[j] == '+' || src[j] == '-')) {
                    ++j;    // 跳过符号
                }

                // 解析指数部分的数字（允许下划线）
                has_underscore = false; // 重置下划线标志
                if (j < src.size() && isdigit(src[j])) {
                    while (j < src.size()) {
                        if (isdigit(src[j])) {
                            has_underscore = false;
                            ++j;
                        } else if (src[j] == '_' && !has_underscore && j + 1 < src.size() && isdigit(src[j + 1])) {
                            has_underscore = true;
                            ++j;
                        } else {
                            break;
                        }
                    }
                } else {
                    // 无效的科学计数法 - 回退到'e'之前的位置
                    j = e_pos;
                }
            }

            // 提取数字字符串并移除所有下划线
            std::string num_str = src.substr(i, j - i);
            num_str.erase(std::remove(num_str.begin(), num_str.end(), '_'), num_str.end());

            tokens.emplace_back(kiz::TokenType::Number, num_str, line, start_col);
            col += (j - i);
            i = j;
        } else if (src[i] == '(') {
            tokens.emplace_back(kiz::TokenType::LParen, "(", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == ')') {
            tokens.emplace_back(kiz::TokenType::RParen, ")", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == ';') {
            tokens.emplace_back(kiz::TokenType::Semicolon, ";", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '"' || src[i] == '\'') {
            char quote_type = src[i];
            size_t j = i + 1;
            std::string str_content;

            while (j < src.size() && src[j] != quote_type) {
                if (src[j] == '\\' && j + 1 < src.size()) {
                    // Handle escape sequences
                    char next = src[j + 1];
                    switch (next) {
                        case 'n':
                            str_content += '\n';
                            break;
                        case 't':
                            str_content += '\t';
                            break;
                        case 'r':
                            str_content += '\r';
                            break;
                        case '\\':
                            str_content += '\\';
                            break;
                        case '"':
                            str_content += '"';
                            break;
                        case '\'':
                            str_content += '\'';
                            break;
                        default:
                            str_content += '\\';
                            str_content += next;
                            break;
                    }
                    j += 2;
                } else {
                    str_content += src[j];
                    j++;
                }
            }

            if (j >= src.size()) {// Unterminated string
                throw StdLibException("Error: Unterminated string literal at line " + line);
                i = src.size();// Stop tokenizing
            } else {
                tokens.emplace_back(kiz::TokenType::String, str_content, line, start_col);
                col += (j - i + 1);
                i = j + 1;
            }
        } else if (src[i] == '+') {
            tokens.emplace_back(kiz::TokenType::Plus, "+", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '-') {
            tokens.emplace_back(kiz::TokenType::Minus, "-", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '*') {
            tokens.emplace_back(kiz::TokenType::Star, "*", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '\\') {
            tokens.emplace_back(kiz::TokenType::Backslash, "\\", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '/' && i + 1 < src.size() && src[i + 1] == '/') {
            // Single line comment with //
            size_t j = i + 2;
            while (j < src.size() && src[j] != '\n') ++j;
            i = j;
            continue;
        } else if (src[i] == '/' && i + 1 < src.size() && src[i + 1] == '*') {
            // Block comment with /* */
            size_t j = i + 2;
            while (j + 1 < src.size()) {
                if (src[j] == '*' && src[j + 1] == '/') {
                    j += 2;
                    break;
                }
                if (src[j] == '\n') {
                    line++;
                    col = 1;
                } else {
                    col++;
                }
                j++;
            }
            i = j;
            continue;
        } else if (src[i] == '/') {
            tokens.emplace_back(kiz::TokenType::Slash, "/", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '%') {
            tokens.emplace_back(kiz::TokenType::Percent, "%", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '^') {
            tokens.emplace_back(kiz::TokenType::Caret, "^", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '!') {
            tokens.emplace_back(kiz::TokenType::Bang, "!", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '{') {
            tokens.emplace_back(kiz::TokenType::LBrace, "{", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '}') {
            if (
                tokens.back().type != kiz::TokenType::Semicolon
                and tokens.back().type != kiz::TokenType::Comma
            ) {
                tokens.emplace_back(kiz::TokenType::Semicolon, ";", line, start_col);
            }
            tokens.emplace_back(kiz::TokenType::RBrace, "}", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '[') {
            tokens.emplace_back(kiz::TokenType::LBracket, "[", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == ']') {
            tokens.emplace_back(kiz::TokenType::RBracket, "]", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '|') {
            tokens.emplace_back(kiz::TokenType::Pipe, "|", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == ',') {
            tokens.emplace_back(kiz::TokenType::Comma, ",", line, start_col);
            ++i;
            ++col;
        } else if (src[i] == '.') {
            ++i;
            ++col;
            if (src[i] == '.') {
                ++i;
                ++col;
                ++i;
                ++col;
                tokens.emplace_back(kiz::TokenType::TripleDot, "...", line, start_col);
            }
            else {
                tokens.emplace_back(kiz::TokenType::Dot, ".", line, start_col);
            }
        } else {
            throw StdLibException("Unknown token '"+std::string(1, src[i]) + "'");
        }
    }
    tokens.emplace_back(kiz::TokenType::EndOfFile, "", line, col);
    return tokens;
}