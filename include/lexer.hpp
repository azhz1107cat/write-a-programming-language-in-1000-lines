/**
 * @file lexer.hpp
 * @brief 词法分析器（Lexer）核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once  
#include <string>
#include <vector>
#include <utility>
#include <cstddef>
#include <unordered_map>

namespace kiz {


// Token 类型与结构体
enum class TokenType {
    // 关键字
    Var, Func, If, Else, While, Return, Import, Break, Continue, Dict,
    True, False, Null, End,
    // 标识符
    Identifier,
    // 赋值运算符
    Assign,
    // 字面量
    Number, String,
    // 分隔符
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Comma, Dot, TripleDot, Semicolon,
    // 运算符
    ExclamationMark, Plus, Minus, Star, Slash, Backslash,
    Percent, Caret, Bang, Equal, NotEqual,
    Less, LessEqual, Greater, GreaterEqual, Pipe,
    FatArrow, ThinArrow, DoubleColon, Not, And, Or, Is, In,
    // 特殊标记
    EndOfFile, EndOfLine, Unknown
};

struct Token {
    TokenType type;
    std::string text;
    size_t lineno;
    size_t column;
    std::unordered_map<std::string, size_t> other_info{};
};

// 词法分析器类
class Lexer {
    std::vector<Token> tokens;
public:
    std::vector<Token> tokenize(const std::string& file_path, const std::string& src);
};

}  // namespace kiz