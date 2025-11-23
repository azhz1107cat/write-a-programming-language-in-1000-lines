/**
 * @file lexer.cpp
 * @brief 词法分析器（Lexer）核心实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "../../include/lexer.hpp"
#include "../include/kiz.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <map>
#include <stdexcept>

#include "util/error_reporter.hpp"

namespace kiz {

static std::map<std::string, TokenType> keywords;
static bool keywords_registered = false;

void register_keywords() {
    if (keywords_registered) return;
    keywords["fn"] = TokenType::Func;
    keywords["if"] = TokenType::If;
    keywords["else"] = TokenType::Else;
    keywords["while"] = TokenType::While;
    keywords["return"] = TokenType::Return;
    keywords["import"] = TokenType::Import;
    keywords["break"] = TokenType::Break;
    keywords["next"] = TokenType::Next;
    keywords["dict"] = TokenType::Dict;
    keywords["end"] = TokenType::End;
    keywords["true"] = TokenType::True;
    keywords["false"] = TokenType::False;
    keywords["null"] = TokenType::Null;

    keywords_registered = true;
}

std::vector<Token> Lexer::tokenize(const std::string& src) {
    std::vector<Token> tokens;
    size_t pos = 0;
    size_t lineno = 1;
    size_t col = 1;
    register_keywords();
    DEBUG_OUTPUT("tokenize the src txt...");
    while (pos < src.size()) {
        if (src[pos] == '\n') {
            if ( !tokens.empty() ) {
                if (tokens.back().type == TokenType::Backslash) {
                    tokens.pop_back();
                }
                tokens.emplace_back(TokenType::EndOfLine, "\n", lineno, col);
            }
            ++lineno;
            col = 1;
            ++pos;
            continue;
        }
        if (isspace(src[pos])) {
            ++col;
            ++pos;
            continue;
        }
        size_t start_col = col;

        if (isalpha(src[pos]) || src[pos] == '_') {
            size_t j = pos + 1;
            while (j < src.size() && (isalnum(src[j]) || src[j] == '_')) ++j;
            std::string ident = src.substr(pos, j - pos);
            auto type = TokenType::Identifier;
            if (keywords.contains(ident)) type = keywords[ident];
            tokens.emplace_back(type, ident, lineno, start_col);
            col += (j - pos);
            pos = j;
        } else if (src[pos] == '=' && pos + 1 < src.size() && src[pos + 1] == '>') {
            tokens.emplace_back(TokenType::FatArrow, "=>", lineno, start_col);
            pos += 2;
            col += 2;
        }
        else if (src[pos] == '-' && pos + 1 < src.size() && src[pos + 1] == '>') {
            tokens.emplace_back(TokenType::ThinArrow, "->", lineno, start_col);
            pos += 2;
            col += 2;
        } else if (src[pos] == '=' && pos + 1 < src.size() && src[pos + 1] == '=') {
            tokens.emplace_back(TokenType::Equal, "==", lineno, start_col);
            pos += 2;
            col += 2;
        } else if (src[pos] == '!' && pos + 1 < src.size() && src[pos + 1] == '=') {
            tokens.emplace_back(TokenType::NotEqual, "!=", lineno, start_col);
            pos += 2;
            col += 2;
        } else if (src[pos] == '!') {
            tokens.emplace_back(TokenType::ExclamationMark, "!", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '<' && pos + 1 < src.size() && src[pos + 1] == '=') {
            tokens.emplace_back(TokenType::LessEqual, "<=", lineno, start_col);
            pos += 2;
            col += 2;
        } else if (src[pos] == '>' && pos + 1 < src.size() && src[pos + 1] == '=') {
            tokens.emplace_back(TokenType::GreaterEqual, ">=", lineno, start_col);
            pos += 2;
            col += 2;
        } else if (src[pos] == '<') {
            tokens.emplace_back(TokenType::Less, "<", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '>') {
            tokens.emplace_back(TokenType::Greater, ">", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == ':') {
            ++pos;
            ++col;
            if (src[pos] == ':') {
                ++pos;
                ++col;
                tokens.emplace_back(TokenType::DoubleColon, "::", lineno, start_col);
            }
        } else if (src[pos] == '=') {
            tokens.emplace_back(TokenType::Assign, "=", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '>') {
            tokens.emplace_back(TokenType::Greater, ">", lineno, start_col);
            ++pos;
            ++col;
        } else if (isdigit(src[pos]) || (src[pos] == '.' && pos + 1 < src.size() && isdigit(src[pos + 1]))) {
            size_t j = pos;
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
                } else if (src[j] == '_' && !has_underscore && j > pos && j + 1 < src.size() && isdigit(src[j + 1])) {
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
            std::string num_str = src.substr(pos, j - pos);
            num_str.erase(std::remove(num_str.begin(), num_str.end(), '_'), num_str.end());

            tokens.emplace_back(TokenType::Number, num_str, lineno, start_col);
            col += (j - pos);
            pos = j;
        } else if (src[pos] == '(') {
            tokens.emplace_back(TokenType::LParen, "(", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == ')') {
            tokens.emplace_back(TokenType::RParen, ")", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == ';') {
            tokens.emplace_back(TokenType::Semicolon, ";", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '"' || src[pos] == '\'') {
            char quote_type = src[pos];
            size_t j = pos + 1;
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
                assert("Error: Unterminated string literal at line " + lineno);
                pos = src.size();// Stop tokenizing
            } else {
                tokens.emplace_back(TokenType::String, str_content, lineno, start_col);
                col += (j - pos + 1);
                pos = j + 1;
            }
        } else if (src[pos] == '+') {
            tokens.emplace_back(TokenType::Plus, "+", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '-') {
            tokens.emplace_back(TokenType::Minus, "-", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '*') {
            tokens.emplace_back(TokenType::Star, "*", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '\\') {
            tokens.emplace_back(TokenType::Backslash, "\\", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '/' && pos + 1 < src.size() && src[pos + 1] == '/') {
            // Single line comment with //
            size_t j = pos + 2;
            while (j < src.size() && src[j] != '\n') ++j;
            pos = j;
            continue;
        } else if (src[pos] == '/' && pos + 1 < src.size() && src[pos + 1] == '*') {
            // Block comment with /* */
            size_t j = pos + 2;
            while (j + 1 < src.size()) {
                if (src[j] == '*' && src[j + 1] == '/') {
                    j += 2;
                    break;
                }
                if (src[j] == '\n') {
                    lineno++;
                    col = 1;
                } else {
                    col++;
                }
                j++;
            }
            pos = j;
            continue;
        } else if (src[pos] == '/') {
            tokens.emplace_back(TokenType::Slash, "/", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '%') {
            tokens.emplace_back(TokenType::Percent, "%", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '^') {
            tokens.emplace_back(TokenType::Caret, "^", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '!') {
            tokens.emplace_back(TokenType::Bang, "!", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '{') {
            tokens.emplace_back(TokenType::LBrace, "{", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '}') {
            if (
                tokens.back().type != TokenType::Semicolon
                and tokens.back().type != TokenType::Comma
            ) {
                tokens.emplace_back(TokenType::Semicolon, ";", lineno, start_col);
            }
            tokens.emplace_back(TokenType::RBrace, "}", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '[') {
            tokens.emplace_back(TokenType::LBracket, "[", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == ']') {
            tokens.emplace_back(TokenType::RBracket, "]", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '|') {
            tokens.emplace_back(TokenType::Pipe, "|", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == ',') {
            tokens.emplace_back(TokenType::Comma, ",", lineno, start_col);
            ++pos;
            ++col;
        } else if (src[pos] == '.') {
            ++pos;
            ++col;
            if (src[pos] == '.') {
                ++pos;
                ++col;
                ++pos;
                ++col;
                tokens.emplace_back(TokenType::TripleDot, "...", lineno, start_col);
            }
            else {
                tokens.emplace_back(TokenType::Dot, ".", lineno, start_col);
            }
        } else {
            util::ErrorInfo err = {
                "SyntaxError",
                "Unknown character '"+std::string(1, src[pos]) + "'",
                1
            };
            util::error_reporter(file_path, lineno, lineno,
                start_col, start_col, err);
        }
    }
    tokens.emplace_back(TokenType::EndOfFile, "", lineno, col);
    return tokens;
}


}  // namespace kiz