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

namespace kiz {

/**
 * @enum TokenType
 * @brief 词法单元（Token）的类型枚举
 * 
 * 按功能分类：关键字、标识符、字面量、运算符、分隔符、特殊标记。
 */
enum class TokenType {
    // 关键字（Keyword）
    Var,        ///< 变量声明关键字（如 `var x = 1`）
    Func,       ///< 函数声明关键字（如 `func add(a, b) { ... }`）
    If,         ///< 条件判断关键字（如 `if (cond) { ... }`）
    Else,       ///< 条件分支关键字（如 `else { ... }`）
    While,      ///< 循环关键字（如 `while (cond) { ... }`）
    Return,     ///< 函数返回关键字（如 `return 42`）
    Import,     ///< 模块导入关键字（如 `import module`）
    Break,      ///< 循环中断关键字（如 `break`）
    Continue,   ///< 循环继续关键字（如 `continue`）
    Dict,       ///< 字典类型关键字（如 `dict { key: val }`）
    True,       ///< 布尔真值（如 `true`）
    False,      ///< 布尔假值（如 `false`）
    Null,       ///< 空值（如 `null`）

    // 标识符（Identifier）：变量名、函数名、模块名等
    Identifier, ///< 标识符（如 `x`、`add`、`user_name`）

    // 赋值运算符（Assignment）
    Assign,     ///< 赋值运算符（`=`，如 `x = 5`）

    // 字面量（Literal）：直接量值
    Number,     ///< 数字字面量（如 `123`、`3.14`、`-45`）
    String,     ///< 字符串字面量（如 `"hello"`、`'world'`，需支持转义）

    // 分隔符（Separator）：括号、逗号、分号等
    LParen,     ///< 左圆括号（`(`，如函数参数列表开始）
    RParen,     ///< 右圆括号（`)`，如函数参数列表结束）
    LBrace,     ///< 左花括号（`{`，如代码块开始）
    RBrace,     ///< 右花括号（`}`，如代码块结束）
    LBracket,   ///< 左方括号（`[`，如数组/字典索引开始）
    RBracket,   ///< 右方括号（`]`，如数组/字典索引结束）
    Comma,      ///< 逗号（`,`，如参数分隔 `a, b`）
    Dot,        ///< 点号（`.`，如属性访问 `obj.prop`）
    Semicolon,  ///< 分号（`;`，如语句结束标记）

    // 运算符（Operator）：算术、比较、逻辑等
    ExclamationMark, ///< 感叹号（`!`，如逻辑非 `!cond`）
    Plus,            ///< 加号（`+`，如加法 `a + b`、正号 `+5`）
    Minus,           ///< 减号（`-`，如减法 `a - b`、负号 `-3`）
    Star,            ///< 星号（`*`，如乘法 `a * b`）
    Slash,           ///< 斜杠（`/`，如除法 `a / b`）
    Backslash,       ///< 反斜杠（`\`，如字符串转义 `\"`）
    Percent,         ///< 百分号（`%`，如取模 `a % b`）
    Caret,           ///< 脱字符（`^`，如按位异或 `a ^ b`）
    Bang,            ///< 井号（`#`，如注释开始 `# 这是注释` 或特殊标记）
    Equal,           ///< 双等号（`==`，如相等比较 `a == b`）
    NotEqual,        ///< 不等号（`!=`，如不等比较 `a != b`）
    Less,            ///< 小于号（`<`，如 `a < b`）
    LessEqual,       ///< 小于等于号（`<=`，如 `a <= b`）
    Greater,         ///< 大于号（`>`，如 `a > b`）
    GreaterEqual,    ///< 大于等于号（`>=`，如 `a >= b`）
    Pipe,            ///< 竖线（`|`，如按位或 `a | b`、逻辑或 `a || b`）
    FatArrow,        ///< 胖箭头（`=>`，如匿名函数 `(a) => a*2`）
    ThinArrow,       ///< 瘦箭头（`->`，如函数返回类型 `func add() -> int`）

    // 特殊标记（Special）
    EndOfFile,   ///< 文件结束标记（EOF，标识源代码解析完成）
    EndOfLine,   ///< 行结束标记（EOL，可选：用于行内语法校验）
    Unknown      ///< 未知标记（如无法识别的字符 `@`、`$`，用于报错）
};

/**
 * @struct Token
 * @brief 词法单元结构体：存储单个Token的完整信息
 * 
 * 包含Token的类型、原始文本、位置（行号/列号），位置信息用于语法分析报错时定位源代码位置，
 * 行号/列号均从 1 开始，跨多行的Token（如多行字符串）需记录开始和结束行号。
 */
struct Token {
    TokenType type;          ///< Token类型（如 Keyword::If、Literal::Number）
    std::string text;        ///< Token原始文本（如 `if`、`123`、`==`）
    size_t lineno_start;     ///< Token开始的行号（从1开始）
    size_t lineno_end;       ///< Token结束的行号（从1开始，单行Token与lineno_start相同）
    size_t column_start;     ///< Token开始的列号（从1开始）
    size_t column_end;       ///< Token结束的列号（从1开始）

    /**
     * @brief Token构造函数（单行Token专用）
     * @param t Token类型
     * @param txt Token原始文本（使用std::move避免拷贝开销）
     * @param lineno Token所在行号（开始=结束）
     * @param col_start Token开始列号
     * @param col_end Token结束列号
     */
    Token(TokenType t, std::string txt, size_t lineno, size_t col_start, size_t col_end)
        : type(t)
        , text(std::move(txt))
        , lineno_start(lineno)
        , lineno_end(lineno)  // 单行Token：结束行=开始行
        , column_start(col_start)
        , column_end(col_end) {}

    /**
     * @brief Token构造函数（跨多行Token专用，如多行字符串）
     * @param t Token类型
     * @param txt Token原始文本
     * @param lineno_s 开始行号
     * @param lineno_e 结束行号
     * @param col_s 开始列号
     * @param col_e 结束列号
     */
    Token(TokenType t, std::string txt, size_t lineno_s, size_t lineno_e, size_t col_s, size_t col_e)
        : type(t)
        , text(std::move(txt))
        , lineno_start(lineno_s)
        , lineno_end(lineno_e)
        , column_start(col_s)
        , column_end(col_e) {}
};

/**
 * @class Lexer
 * @brief 词法分析器类：将源代码字符串转换为Token序列
 * 
 * 核心接口为 tokenize()，内部通过私有辅助函数实现字符读取、空白跳过、关键字判断等逻辑，
 * 封装解析状态（当前位置、行号、列号），确保外部无法直接修改解析过程，符合封装原则。
 */
class Lexer {
public:
    /**
     * @brief 对源代码进行分词，生成Token序列
     * @param src 源代码字符串（如从文件读取的代码）
     * @return std::vector<Token> 分词后的Token列表，末尾包含EndOfFile标记
     * @note 若遇到无法识别的字符，会生成Unknown类型的Token（含位置信息），不中断分词流程
     */
    std::vector<Token> tokenize(const std::string& src);
};

}  // namespace kiz