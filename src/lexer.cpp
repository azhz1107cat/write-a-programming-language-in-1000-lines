/**
 * @file lexer.cpp
 * @brief 词法分析器（Lexer）核心实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "../include/lexer.hpp"
#include <cctype>
#include <stdexcept>
#include <algorithm>

namespace kiz {

// 初始化静态关键字映射
std::unordered_map<std::string, TokenType> Lexer::keywords_;
bool Lexer::keywords_inited_ = false;

// 辅助函数实现
void Lexer::init(const std::string& src) {
    // 初始化 FSM 上下文
    current_state_ = LexerState::Initial;
    src_ = &src;
    idx_ = 0;
    line_ = 1;
    col_ = 1;
    tokens_.clear();
    token_buf_.clear();
    token_line_start_ = 0;
    token_col_start_ = 0;

    // 初始化关键字（仅一次）
    if (!keywords_inited_) {
        keywords_ = {
            {"var", TokenType::Var}, {"func", TokenType::Func}, {"if", TokenType::If},
            {"else", TokenType::Else}, {"while", TokenType::While}, {"return", TokenType::Return},
            {"import", TokenType::Import}, {"break", TokenType::Break}, {"continue", TokenType::Continue},
            {"dict", TokenType::Dict}, {"true", TokenType::True}, {"false", TokenType::False},
            {"null", TokenType::Null}
        };
        keywords_inited_ = true;
    }
}

void Lexer::transition(LexerState next_state) {
    current_state_ = next_state;
}

void Lexer::emit_token(TokenType type) {
    // 生成单行 Token（复用缓存的起始位置）
    tokens_.emplace_back(
        type, token_buf_,
        token_line_start_, token_col_start_,
        line_, col_ - 1  // 结束列 = 当前列 - 1（已读取完 Token 字符）
    );
    // 清空缓存
    token_buf_.clear();
}

void Lexer::emit_token_multi_line(TokenType type, size_t line_end, size_t col_end) {
    // 生成跨多行 Token（如多行字符串）
    tokens_.emplace_back(
        type, token_buf_,
        token_line_start_, line_end,
        token_col_start_, col_end
    );
    token_buf_.clear();
}

bool Lexer::is_keyword(const std::string& ident) {
    return keywords_.count(ident) > 0;
}

char Lexer::peek() const {
    // 查看下一个字符（不移动索引）
    return (idx_ + 1 < src_->size()) ? (*src_)[idx_ + 1] : '\0';
}

void Lexer::skip_whitespace() {
    // 跳过空格、制表符（不包括换行，换行在初始态单独处理）
    while (idx_ < src_->size() && (std::isspace((*src_)[idx_]) && (*src_)[idx_] != '\n')) {
        idx_++;
        col_++;
    }
}

// 状态处理函数实现
void Lexer::handle_initial_state() {
    skip_whitespace(); // 跳过初始态的空白字符
    if (idx_ >= src_->size()) {
        transition(LexerState::Terminated);
        return;
    }

    char c = (*src_)[idx_];
    switch (true) {
        // 进入标识符/关键字态：字母或下划线开头
        case (std::isalpha(c) || c == '_'):
            token_buf_.push_back(c);
            token_line_start_ = line_;
            token_col_start_ = col_;
            idx_++;
            col_++;
            transition(LexerState::Identifier);
            break;

        // 进入数字态：数字或小数点开头（小数点后需有数字）
        case (std::isdigit(c) || (c == '.' && std::isdigit(peek()))):
            token_buf_.push_back(c);
            token_line_start_ = line_;
            token_col_start_ = col_;
            idx_++;
            col_++;
            transition(LexerState::Number);
            break;

        // 进入字符串态：单/双引号
        case (c == '"' || c == '\''):
            token_buf_.clear(); // 字符串内容不含引号，仅缓存字符
            token_line_start_ = line_;
            token_col_start_ = col_;
            idx_++;
            col_++;
            transition(LexerState::String);
            break;

        // 进入运算符态：可能是多字符运算符（=, !, <, >, -, | 等）
        case (c == '=' || c == '!' || c == '<' || c == '>' || c == '-' || 
              c == '+' || c == '*' || c == '/' || c == '%' || c == '^' || 
              c == '|' || c == '.'):
            token_buf_.push_back(c);
            token_line_start_ = line_;
            token_col_start_ = col_;
            idx_++;
            col_++;
            transition(LexerState::Operator);
            break;

        // 进入注释态：// 或 /*
        case (c == '/' && peek() == '/'):
            idx_ += 2;
            col_ += 2;
            transition(LexerState::CommentSingle);
            break;
        case (c == '/' && peek() == '*'):
            idx_ += 2;
            col_ += 2;
            transition(LexerState::CommentBlock);
            break;

        // 单字符分隔符：直接生成 Token
        case (c == '('):
            token_buf_ = "(";
            emit_token(TokenType::LParen);
            idx_++;
            col_++;
            break;
        case (c == ')'):
            token_buf_ = ")";
            emit_token(TokenType::RParen);
            idx_++;
            col_++;
            break;
        case (c == '{'):
            token_buf_ = "{";
            emit_token(TokenType::LBrace);
            idx_++;
            col_++;
            break;
        case (c == '}'):
            token_buf_ = "}";
            emit_token(TokenType::RBrace);
            idx_++;
            col_++;
            break;
        case (c == '['):
            token_buf_ = "[";
            emit_token(TokenType::LBracket);
            idx_++;
            col_++;
            break;
        case (c == ']'):
            token_buf_ = "]";
            emit_token(TokenType::RBracket);
            idx_++;
            col_++;
            break;
        case (c == ','):
            token_buf_ = ",";
            emit_token(TokenType::Comma);
            idx_++;
            col_++;
            break;
        case (c == ';'):
            token_buf_ = ";";
            emit_token(TokenType::Semicolon);
            idx_++;
            col_++;
            break;
        case (c == '#'): // Bang 符号
            token_buf_ = "#";
            emit_token(TokenType::Bang);
            idx_++;
            col_++;
            break;

        // 换行符：生成 EndOfLine
        case (c == '\n'):
            // 原逻辑：若最后是反斜杠，移除反斜杠不生成 EOL
            if (!tokens_.empty() && tokens_.back().type == TokenType::Backslash) {
                tokens_.pop_back();
            } else {
                token_buf_ = "\n";
                emit_token(TokenType::EndOfLine);
            }
            idx_++;
            line_++;
            col_ = 1;
            break;

        // 未知字符
        default:
            token_buf_ = std::string(1, c);
            transition(LexerState::Unknown);
            break;
    }
}

void Lexer::handle_identifier_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：生成标识符/关键字 Token
        TokenType type = is_keyword(token_buf_) ? keywords_[token_buf_] : TokenType::Identifier;
        emit_token(type);
        transition(LexerState::Terminated);
        return;
    }

    char c = (*src_)[idx_];
    if (std::isalnum(c) || c == '_') {
        // 继续读取标识符字符
        token_buf_.push_back(c);
        idx_++;
        col_++;
    } else {
        // 遇到非标识符字符：生成 Token，回到初始态
        TokenType type = is_keyword(token_buf_) ? keywords_[token_buf_] : TokenType::Identifier;
        emit_token(type);
        transition(LexerState::Initial);
    }
}

void Lexer::handle_number_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：处理数字（移除下划线）
        token_buf_.erase(std::remove(token_buf_.begin(), token_buf_.end(), '_'), token_buf_.end());
        emit_token(TokenType::Number);
        transition(LexerState::Terminated);
        return;
    }

    char c = (*src_)[idx_];
    switch (true) {
        case (std::isdigit(c)):
            // 读取数字
            token_buf_.push_back(c);
            idx_++;
            col_++;
            break;
        case (c == '.' && token_buf_.find('.') == std::string::npos):
            // 读取小数点（仅允许一个）
            token_buf_.push_back(c);
            idx_++;
            col_++;
            transition(LexerState::NumberDot);
            break;
        case (c == 'e' || c == 'E'):
            // 读取科学计数法的 e/E
            token_buf_.push_back(c);
            idx_++;
            col_++;
            transition(LexerState::NumberExp);
            break;
        case (c == '_' && std::isdigit(peek())):
            // 读取下划线（仅允许数字间的分隔）
            token_buf_.push_back(c);
            idx_++;
            col_++;
            break;
        default:
            // 遇到非数字字符：生成 Number Token
            token_buf_.erase(std::remove(token_buf_.begin(), token_buf_.end(), '_'), token_buf_.end());
            emit_token(TokenType::Number);
            transition(LexerState::Initial);
            break;
    }
}

void Lexer::handle_number_dot_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：小数点结尾视为无效，按未知处理
        transition(LexerState::Unknown);
        return;
    }

    char c = (*src_)[idx_];
    if (std::isdigit(c)) {
        // 继续读取小数点后的数字
        token_buf_.push_back(c);
        idx_++;
        col_++;
    } else if (c == 'e' || c == 'E') {
        // 进入科学计数法态
        token_buf_.push_back(c);
        idx_++;
        col_++;
        transition(LexerState::NumberExp);
    } else {
        // 遇到非数字：生成 Number Token
        token_buf_.erase(std::remove(token_buf_.begin(), token_buf_.end(), '_'), token_buf_.end());
        emit_token(TokenType::Number);
        transition(LexerState::Initial);
    }
}

void Lexer::handle_number_exp_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：e 结尾视为无效
        transition(LexerState::Unknown);
        return;
    }

    char c = (*src_)[idx_];
    if (std::isdigit(c)) {
        // 读取指数部分数字
        token_buf_.push_back(c);
        idx_++;
        col_++;
    } else if (c == '+' || c == '-') {
        // 读取指数符号
        token_buf_.push_back(c);
        idx_++;
        col_++;
        transition(LexerState::NumberExpSign);
    } else {
        // 无效指数：回退到 e 之前，生成数字 Token
        token_buf_.pop_back(); // 移除 e
        token_buf_.erase(std::remove(token_buf_.begin(), token_buf_.end(), '_'), token_buf_.end());
        emit_token(TokenType::Number);
        transition(LexerState::Initial);
    }
}

void Lexer::handle_number_exp_sign_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：符号结尾视为无效
        transition(LexerState::Unknown);
        return;
    }

    char c = (*src_)[idx_];
    if (std::isdigit(c)) {
        // 读取指数符号后的数字
        token_buf_.push_back(c);
        idx_++;
        col_++;
    } else {
        // 无效指数：回退符号和 e，生成数字 Token
        token_buf_.pop_back(); // 移除符号
        token_buf_.pop_back(); // 移除 e
        token_buf_.erase(std::remove(token_buf_.begin(), token_buf_.end(), '_'), token_buf_.end());
        emit_token(TokenType::Number);
        transition(LexerState::Initial);
    }
}

void Lexer::handle_string_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：未闭合字符串，抛错
        std::assert(false && "Unterminated string at line " + std::to_string(line_));
    }

    char c = (*src_)[idx_];
    if (c == '"' || c == '\'') {
        // 遇到闭合引号：生成 String Token
        emit_token(TokenType::String);
        idx_++;
        col_++;
        transition(LexerState::Initial);
    } else if (c == '\\' && idx_ + 1 < src_->size()) {
        // 处理转义字符
        char next = (*src_)[idx_ + 1];
        switch (next) {
            case 'n': token_buf_.push_back('\n'); break;
            case 't': token_buf_.push_back('\t'); break;
            case 'r': token_buf_.push_back('\r'); break;
            case '\\': token_buf_.push_back('\\'); break;
            case '"': token_buf_.push_back('"'); break;
            case '\'': token_buf_.push_back('\''); break;
            default: token_buf_.push_back('\\'); token_buf_.push_back(next); break;
        }
        idx_ += 2;
        col_ += 2;
    } else {
        // 读取普通字符
        token_buf_.push_back(c);
        if (c == '\n') {
            // 多行字符串：更新行号列号
            line_++;
            col_ = 1;
        } else {
            col_++;
        }
        idx_++;
    }
}

void Lexer::handle_operator_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：生成单字符运算符 Token
        char op = token_buf_[0];
        TokenType type = [op]() {
            switch (op) {
                case '=': return TokenType::Assign;
                case '!': return TokenType::ExclamationMark;
                case '<': return TokenType::Less;
                case '>': return TokenType::Greater;
                case '-': return TokenType::Minus;
                case '+': return TokenType::Plus;
                case '*': return TokenType::Star;
                case '/': return TokenType::Slash;
                case '%': return TokenType::Percent;
                case '^': return TokenType::Caret;
                case '|': return TokenType::Pipe;
                case '.': return TokenType::Dot;
                default: return TokenType::Unknown;
            }
        }();
        emit_token(type);
        transition(LexerState::Terminated);
        return;
    }

    char c = (*src_)[idx_];
    std::string current_op = token_buf_ + c;
    // 判断是否为多字符运算符
    TokenType multi_type = TokenType::Unknown;
    if (current_op == "==") multi_type = TokenType::Equal;
    else if (current_op == "!=") multi_type = TokenType::NotEqual;
    else if (current_op == "<=") multi_type = TokenType::LessEqual;
    else if (current_op == ">=") multi_type = TokenType::GreaterEqual;
    else if (current_op == "=>") multi_type = TokenType::FatArrow;
    else if (current_op == "->") multi_type = TokenType::ThinArrow;
    else if (current_op == "::") multi_type = TokenType::DoubleColon;
    else if (current_op == "..") {
        // 处理三字符运算符 ...
        if (peek() == '.') {
            token_buf_ += "..";
            idx_ += 2;
            col_ += 2;
            emit_token(TokenType::TripleDot);
            transition(LexerState::Initial);
            return;
        }
    }

    if (multi_type != TokenType::Unknown) {
        // 多字符运算符：更新缓存，生成 Token
        token_buf_ = current_op;
        emit_token(multi_type);
        idx_++;
        col_++;
        transition(LexerState::Initial);
    } else {
        // 单字符运算符：生成 Token
        char op = token_buf_[0];
        TokenType type = [op]() {
            switch (op) {
                case '=': return TokenType::Assign;
                case '!': return TokenType::ExclamationMark;
                case '<': return TokenType::Less;
                case '>': return TokenType::Greater;
                case '-': return TokenType::Minus;
                case '+': return TokenType::Plus;
                case '*': return TokenType::Star;
                case '/': return TokenType::Slash;
                case '%': return TokenType::Percent;
                case '^': return TokenType::Caret;
                case '|': return TokenType::Pipe;
                case '.': return TokenType::Dot;
                default: return TokenType::Unknown;
            }
        }();
        emit_token(type);
        transition(LexerState::Initial);
    }
}

void Lexer::handle_comment_single_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：结束单行注释
        transition(LexerState::Terminated);
        return;
    }

    char c = (*src_)[idx_];
    if (c == '\n') {
        // 换行：结束单行注释，处理换行
        idx_++;
        line_++;
        col_ = 1;
        transition(LexerState::Initial);
    } else {
        // 跳过注释内容
        idx_++;
        col_++;
    }
}

void Lexer::handle_comment_block_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：未闭合多行注释，抛错
        std::assert(false && "Unterminated block comment at line " + std::to_string(line_));
    }

    char c = (*src_)[idx_];
    if (c == '*') {
        // 可能到达注释结尾：进入结束态
        idx_++;
        col_++;
        transition(LexerState::CommentBlockEnd);
    } else {
        // 跳过注释内容（更新行号列号）
        if (c == '\n') {
            line_++;
            col_ = 1;
        } else {
            col_++;
        }
        idx_++;
    }
}

void Lexer::handle_comment_block_end_state() {
    if (idx_ >= src_->size()) {
        // 到达 EOF：未闭合多行注释，抛错
        std::assert(false && "Unterminated block comment at line " + std::to_string(line_));
    }

    char c = (*src_)[idx_];
    if (c == '/') {
        // 闭合多行注释：回到初始态
        idx_++;
        col_++;
        transition(LexerState::Initial);
    } else if (c == '*') {
        // 仍在注释结尾态：继续等待 /
        idx_++;
        col_++;
    } else {
        // 回到多行注释态，继续读取
        idx_++;
        col_++;
        transition(LexerState::CommentBlock);
    }
}

void Lexer::handle_unknown_state() {
    // 未知字符：抛错
    std::assert(false && "Unknown token '" + token_buf_ + "' at line " + std::to_string(line_) + 
                             ", column " + std::to_string(col_));
}

// 核心分词接口
std::vector<Token> Lexer::tokenize(const std::string& src) {
    init(src); // 初始化 FSM

    // FSM 主循环：直到进入终止态
    while (current_state_ != LexerState::Terminated) {
        switch (current_state_) {
            case LexerState::Initial: handle_initial_state(); break;
            case LexerState::Identifier: handle_identifier_state(); break;
            case LexerState::Number: handle_number_state(); break;
            case LexerState::NumberDot: handle_number_dot_state(); break;
            case LexerState::NumberExp: handle_number_exp_state(); break;
            case LexerState::NumberExpSign: handle_number_exp_sign_state(); break;
            case LexerState::String: handle_string_state(); break;
            case LexerState::Operator: handle_operator_state(); break;
            case LexerState::CommentSingle: handle_comment_single_state(); break;
            case LexerState::CommentBlock: handle_comment_block_state(); break;
            case LexerState::CommentBlockEnd: handle_comment_block_end_state(); break;
            case LexerState::Unknown: handle_unknown_state(); break;
            case LexerState::Terminated: break; // 终止态：退出循环
        }
    }

    // 生成 EOF Token
    tokens_.emplace_back(TokenType::EndOfFile, "", line_, col_);
    return tokens_;
}

}  // namespace kiz