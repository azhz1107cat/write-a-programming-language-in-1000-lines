/**
 * @file ast.hpp
 * @brief 抽象语法树（AST）核心定义
 *  
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

namespace kiz {

// AST 基类
struct ASTNode {
    int start_ln = 0;
    int end_ln = 0;
    int start_col = 0;
    int end_col = 0;
    virtual ~ASTNode() = default;
};

// 类型信息
struct TypeInfo {
    std::string type_name;
    std::vector<std::unique_ptr<TypeInfo>> subs;
    TypeInfo(std::string tn, std::vector<std::unique_ptr<TypeInfo>> s)
        : type_name(std::move(tn)), subs(std::move(s)) {}
};

enum class AstType {
    
};

// 表达式基类
struct Expression :  ASTNode {
    std::unique_ptr<TypeInfo> ast_type;
};

// 语句基类
struct Statement :  ASTNode {};

// 字符串字面量
struct StringExpr final :  Expression {
    std::string value;
    LiteralExpr(std::string v)
        : type(t) {}
};

// 数字字面量
struct NumberExpr final :  Expression {
    std::string value;
    LiteralExpr(std::string v)
        : type(t) {}
};

// 数组字面量
struct ArrayExpr final :  Expression {
    std::vector<std::unique_ptr<Expression>> elements;
    explicit ArrayExpr(std::vector<std::unique_ptr<Expression>> elems)
        : elements(std::move(elems)) {}
};

// 标识符
struct IdentifierExpr final :  Expression {
    std::string name;
    explicit IdentifierExpr(std::string  n) : name(std::move(n)) {}
};

// 二元运算
struct BinaryExpr final :  Expression {
    std::string op;
    std::unique_ptr<Expression> left, right;
    BinaryExpr(std::string  o, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
};

// 一元运算
struct UnaryExpr final :  Expression {
    std::string op;
    std::unique_ptr<Expression> operand;
    UnaryExpr(std::string  o, std::unique_ptr<Expression> e)
        : op(std::move(o)), operand(std::move(e)) {}
};

// 变量声明
struct VarDeclStmt final :  Statement {
    std::string name;
    std::unique_ptr<Expression> expr;
    VarDeclStmt(std::string  n, std::unique_ptr<Expression> e)
        : name(std::move(n)), expr(std::move(e)) {}
};

// 赋值
struct AssignStmt final :  Statement {
    std::string name;
    std::unique_ptr<Expression> expr;
    AssignStmt(std::string  n, std::unique_ptr<Expression> e)
        : name(std::move(n)), expr(std::move(e)) {}
};

// 复合语句块
struct BlockStmt final :  Statement {
    std::vector<std::unique_ptr<Statement>> statements{};
    explicit BlockStmt(std::vector<std::unique_ptr<Statement>> s) : statements(std::move(s)) {}
};

// if 语句
struct IfStmt final :  Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStmt> thenBlock;
    std::unique_ptr<BlockStmt> elseBlock;
    IfStmt(std::unique_ptr<Expression> cond, std::unique_ptr<BlockStmt> thenB, std::unique_ptr<BlockStmt> elseB)
        : condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(std::move(elseB)) {}
};

// while 语句
struct WhileStmt final :  Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStmt> body;
    WhileStmt(std::unique_ptr<Expression> cond, std::unique_ptr<BlockStmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// 函数定义
struct FuncDefStmt final :  Statement {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<BlockStmt> body;
    FuncDefStmt(std::string  n, const std::vector<std::string>& p, std::unique_ptr<BlockStmt> b)
        : name(std::move(n)), params(p), body(std::move(b)) {}
};

// 函数调用
struct CallExpr final :  Expression {
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> args;
    CallExpr(std::unique_ptr<Expression> c, std::vector<std::unique_ptr<Expression>> a)
        : callee(std::move(c)), args(std::move(a)) {}
};

// 获取成员
struct GetMemberExpr final :  Expression {
    std::unique_ptr<Expression> father;
    std::unique_ptr<IdentifierExpr> child;
    GetMemberExpr(std::unique_ptr<Expression> f, std::unique_ptr<IdentifierExpr> c)
        :father(std::move(f)), child(std::move(c)) {}
};

// 设置成员
struct SetMemberExpr final :  Expression {
    std::unique_ptr<Expression> g_mem;
    std::unique_ptr<Expression> val;
    SetMemberExpr(std::unique_ptr<Expression> g_mem, std::unique_ptr<Expression> val)
        : g_mem(std::move(g_mem)), val(std::move(val)) {}
};

// 获取项
struct GetItemExpr final :  Expression {
    std::unique_ptr<Expression> father;
    std::vector<std::unique_ptr<Expression>> params;
    GetItemExpr(std::unique_ptr<Expression> f, std::vector<std::unique_ptr<Expression>> p)
        : father(std::move(f)), params(std::move(p)) {}
};

// 声明匿名函数
struct LambdaDeclExpr final :  Expression {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<BlockStmt> body;
    LambdaDeclExpr(std::string n, std::vector<std::string> p, std::unique_ptr<BlockStmt> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
};

// 声明字典
struct DictDeclExpr final :  Expression {
    std::string name;
    std::vector<std::string, std::unique_ptr<Expression>> init_list;
    DictDeclExpr(std::string n, std::vector<std::string, std::unique_ptr<Expression>> i)
        : name(std::move(n)), init_list(std::move(i)) {}
};

// return 语句
struct ReturnStmt final :  Statement {
    std::unique_ptr<Expression> expr;
    explicit ReturnStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

// import 语句
struct ImportStmt final :  Statement {
    std::string path;
    explicit ImportStmt(std::string  p) : path(std::move(m)) {}
};

// 空语句
struct NullStmt final :  Statement {
    NullStmt() = default;
};

// break 语句
struct BreakStmt final :  Statement {
    BreakStmt() = default;
};

// continue 语句
struct ContinueStmt final :  Statement {
    ContinueStmt() = default;
};

// 表达式语句
struct ExprStmt final :  Statement {
    std::unique_ptr<Expression> expr;
    explicit ExprStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

} // namespace kiz