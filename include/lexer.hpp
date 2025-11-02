/**
 * @file lexer.hpp
 * @brief 词法分析器（Lexer）核心定义
 * 
 * 负责将源代码字符串转换为结构化的词法单元（Token）序列，包含：
 * TokenType 枚举：定义所有支持的词法单元类型（关键字、标识符、运算符等）；
 * Token 结构体：存储单个词法单元的类型、文本、位置（行/列）信息；
 * Lexer 类：提供分词接口（tokenize）及内部辅助解析逻辑（空白跳过、关键字判断等）。
 * 
 * 位置信息（行号/列号）从 1 开始计数，便于后续语法分析报错定位；
 * 支持基础语法元素：关键字、标识符、数字、字符串、运算符、分隔符。
 * 
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

// 有限自动状态机（FSM）的状态枚举
/**
 * @enum LexerState
 * @brief FSM 状态：描述分词过程的不同阶段
 */
enum class LexerState {
    Initial,        // 初始态：等待下一个 Token 的开始
    Identifier,     // 标识符/关键字态：读取字母、数字、下划线
    Number,         // 数字态：读取整数、小数、科学计数法
    NumberDot,      // 数字小数点态：已读取小数点，等待后续数字
    NumberExp,      // 数字指数态：已读取 e/E，等待指数部分（正负号/数字）
    NumberExpSign,  // 数字指数符号态：已读取 e后的±，等待指数数字
    String,         // 字符串态：读取引号内字符（含转义）
    Operator,       // 运算符态：处理多字符运算符（如 ==、=>）
    CommentSingle,  // 单行注释态：// 直到换行
    CommentBlock,   // 多行注释态：/* 直到 */
    CommentBlockEnd,// 多行注释结束态：已读取 *，等待 /
    Unknown,        // 未知态：遇到无法识别的字符
    Terminated      // 终止态：分词完成（EOF）
};

// Token 类型与结构体
enum class TokenType {
    // 关键字
    Var, Func, If, Else, While, Return, Import, Break, Continue, Dict,
    True, False, Null,
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
    FatArrow, ThinArrow, DoubleColon,
    // 特殊标记
    EndOfFile, EndOfLine, Unknown
};

struct Token {
    TokenType type;
    std::string text;
    size_t lineno_start;
    size_t lineno_end;
    size_t column_start;
    size_t column_end;

    // 单行 Token 构造
    Token(TokenType t, std::string txt, size_t lineno, size_t col_start, size_t col_end)
        : type(t), text(std::move(txt)), lineno_start(lineno), lineno_end(lineno),
          column_start(col_start), column_end(col_end) {}

    // 跨多行 Token 构造（如多行字符串）
    Token(TokenType t, std::string txt, size_t lineno_s, size_t lineno_e, size_t col_s, size_t col_e)
        : type(t), text(std::move(txt)), lineno_start(lineno_s), lineno_end(lineno_e),
          column_start(col_s), column_end(col_e) {}
};

// 词法分析器类
class Lexer {
public:
    std::vector<Token> tokenize(const std::string& src);

private:
    // FSM 上下文：当前状态、源代码索引、位置信息、Token 缓存
    LexerState current_state_;       // 当前状态
    const std::string* src_;         // 源代码指针（避免拷贝）
    size_t idx_;                     // 当前字符索引
    size_t line_;                    // 当前行号（1-based）
    size_t col_;                     // 当前列号（1-based）
    std::vector<Token> tokens_;      // 最终 Token 结果
    // 临时缓存：当前 Token 的文本、起始位置（用于生成 Token）
    std::string token_buf_;
    size_t token_line_start_;
    size_t token_col_start_;

    // 关键字映射
    static std::unordered_map<std::string, TokenType> keywords_;
    static bool keywords_inited_;

    // 核心s状态处理函数（每个状态对应一个函数
    void init(const std::string& src);          // 初始化 FSM 上下文
    void transition(LexerState next_state);     // 状态转移（更新当前状态）
    void emit_token(TokenType type);            // 生成 Token（清空缓存）
    void emit_token_multi_line(TokenType type, size_t line_end, size_t col_end); // 跨多行 Token
    bool is_keyword(const std::string& ident);  // 判断标识符是否为关键字
    char peek() const;                          // 查看下一个字符（不移动索引）
    void skip_whitespace();                     // 跳过空白字符（空格、制表符）

    // 各状态的核心处理逻辑
    void handle_initial_state();    // 初始态处理
    void handle_identifier_state(); // 标识符/关键字态处理
    void handle_number_state();     // 数字态处理
    void handle_number_dot_state(); // 数字小数点态处理
    void handle_number_exp_state(); // 数字指数态处理
    void handle_number_exp_sign_state(); // 数字指数符号态处理
    void handle_string_state();     // 字符串态处理
    void handle_operator_state();   // 运算符态处理
    void handle_comment_single_state(); // 单行注释态处理
    void handle_comment_block_state();  // 多行注释态处理
    void handle_comment_block_end_state(); // 多行注释结束态处理
    void handle_unknown_state();    // 未知态处理
};

}  // namespace kiz