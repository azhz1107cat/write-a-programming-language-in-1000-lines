/**
 * @file ir_gen.cpp
 * @brief 中间代码生成器（IR Generator）核心实现
 * 从AST生成IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "../../include/ir_gen.hpp"
#include "../../include/ast.hpp"
#include "../../include/models.hpp"
#include "../../include/opcode.hpp"
#include <algorithm>
#include <cassert>

namespace kiz {

// 辅助函数：获取变量名在curr_names中的索引（不存在则添加）
static size_t get_or_add_name(std::vector<std::string>& names, const std::string& name) {
    auto it = std::find(names.begin(), names.end(), name);
    if (it != names.end()) {
        return std::distance(names.begin(), it);
    }
    names.emplace_back(name);
    return names.size() - 1;
}

// 辅助函数：获取常量在curr_const中的索引（不存在则添加）
static size_t get_or_add_const(std::vector<model::Object*>& consts, model::Object* obj) {
    auto it = std::find(consts.begin(), consts.end(), obj);
    if (it != consts.end()) {
        return std::distance(consts.begin(), it);
    }
    obj->make_ref(); // 管理引用计数
    consts.emplace_back(obj);
    return consts.size() - 1;
}

// 处理代码块（生成块内所有语句的IR）
void IRGenerator::gen_block(BlockStmt* block) {
    if (!block) return;
    for (auto& stmt : block->statements) {
        switch (stmt->ast_type) {
            case AstType::VarDeclStmt: {
                // 变量声明：生成初始化表达式IR + 存储变量指令
                auto* var_decl = static_cast<VarDeclStmt*>(stmt.get());
                gen_expr(var_decl->expr.get()); // 生成初始化表达式IR
                size_t name_idx = get_or_add_name(curr_names, var_decl->name);

                curr_code_list.emplace_back(
                    Opcode::SET_LOCAL, 
                    std::vector<size_t>{name_idx},
                    stmt->start_ln, 
                    stmt->end_ln
                );
                break;
            }
            case AstType::ExprStmt: {
                // 表达式语句：生成表达式IR + 弹出结果（避免栈泄漏）
                auto* expr_stmt = static_cast<ExprStmt*>(stmt.get());
                gen_expr(expr_stmt->expr.get());
                curr_code_list.emplace_back(
                    Opcode::POP_TOP, 
                    std::vector<size_t>{},
                    stmt->start_ln, 
                    stmt->end_ln
                );
                break;
            }
            case AstType::IfStmt:
                gen_if(static_cast<IfStmt*>(stmt.get()));
                break;
            case AstType::WhileStmt:
                gen_while(static_cast<WhileStmt*>(stmt.get()));
                break;
            case AstType::ReturnStmt: {
                // 返回语句：生成返回值表达式IR + RET指令
                auto* ret_stmt = static_cast<ReturnStmt*>(stmt.get());
                if (ret_stmt->expr) {
                    gen_expr(ret_stmt->expr.get());
                } else {
                    // 无返回值时压入Nil常量
                    model::Nil* nil = new model::Nil();
                    size_t const_idx = get_or_add_const(curr_const, nil);
                    curr_code_list.emplace_back(
                        Opcode::LOAD_CONST, 
                        std::vector<size_t>{const_idx},
                        stmt->start_ln, 
                        stmt->end_ln
                    );
                }
                curr_code_list.emplace_back(
                    Opcode::RET, 
                    std::vector<size_t>{},
                    stmt->start_ln, 
                    stmt->end_ln
                );
                pbreak;
            }
            case AstType::BreakStmt:
                // Break语句：跳转到循环结束位置（依赖block_stack记录循环出口）
                assert(!block_stack.empty() && "BreakStmt: 无活跃循环块");
                curr_code_list.emplace_back(
                    Opcode::JUMP, 
                    std::vector<size_t>{block_stack.top()},
                    stmt->start_ln, 
                    stmt->end_ln
                );
                break;
            case AstType::ContinueStmt:
                // Continue语句：跳转到循环条件位置（依赖block_stack记录循环入口）
                assert(block_stack.size() >= 2 && "ContinueStmt: 无活跃循环块");
                curr_code_list.emplace_back(
                    Opcode::JUMP, 
                    std::vector<size_t>{block_stack.top()},
                    stmt->start_ln, 
                    stmt->end_ln
                );
                break;
            default:
                assert(false && "gen_block: 未处理的语句类型");
        }
    }
}

model::Module* IRGenerator::gen() {
    // 检查AST根节点有效性（默认模块根为BlockStmt）
    assert(ast && ast->ast_type == AstType::BlockStmt && "gen: AST根节点非BlockStmt");
    auto* root_block = static_cast<BlockStmt*>(ast.get());

    // 初始化模块级代码容器
    curr_code_list.clear();
    curr_names.clear();
    curr_const.clear();
    curr_lineno_map.clear();
    block_stack.empty();

    // 处理模块顶层节点
    gen_mod(root_block);

    // 生成模块代码对象
    model::CodeObject* mod_code = make_code_obj();
    assert(mod_code && "gen: 模块CodeObject创建失败");

    // 初始化模块实例
    model::Module* mod = new model::Module();
    mod->name = "main"; // 默认模块名为main
    mod->code = mod_code;
    mod->code->make_ref(); // 管理CodeObject引用计数

    // 模块属性初始化（添加全局变量映射）
    for (size_t i = 0; i < curr_names.size(); ++i) {
        const std::string& var_name = curr_names[i];
        // 查找变量对应的常量（简化：假设全局变量值存于curr_const）
        if (i < curr_const.size()) {
            model::Object* var_val = curr_const[i];
            var_val->make_ref();
            mod->attrs[var_name] = var_val;
        }
    }

    return mod;
}


void IRGenerator::gen_expr(Expression* expr) {
    assert(expr && "gen_expr: 表达式节点为空");
    switch (expr->ast_type) {
        case AstType::NumberExpr:
            gen_literal(static_cast<NumberExpr*>(expr));
            break;
        case AstType::StringExpr:
            gen_literal(static_cast<StringExpr*>(expr));
            break;
        case AstType::IdentifierExpr: {
            // 标识符：生成LOAD_VAR指令（加载变量值）
            auto* ident = static_cast<IdentifierExpr*>(expr);
            size_t name_idx = get_or_add_name(curr_names, ident->name);
            curr_code_list.emplace_back(
                Opcode::LOAD_VAR, 
                std::vector<size_t>{name_idx},
                expr->start_ln, 
                expr->end_ln
            );
            curr_lineno_map.emplace_back(curr_code_list.size() - 1, expr->start_ln);
            break;
        }
        case AstType::BinaryExpr: {
            // 二元运算：生成左表达式 -> 右表达式 -> 运算指令
            auto* bin_expr = static_cast<BinaryExpr*>(expr);
            gen_expr(bin_expr->left.get());  // 左操作数
            gen_expr(bin_expr->right.get()); // 右操作数（栈中顺序：左在下，右在上）
                
            // 映射运算符到 opcode
            Opcode opc;
            if (bin_expr->op == "+") opc = Opcode::OP_ADD;
            else if (bin_expr->op == "-") opc = Opcode::OP_SUB;
            else if (bin_expr->op == "*") opc = Opcode::OP_MUL;
            else if (bin_expr->op == "/") opc = Opcode::OP_DIV;
            else if (bin_expr->op == "%") opc = Opcode::OP_MOD;
            else if (bin_expr->op == "^") opc = Opcode::OP_POW;
            else if (bin_expr->op == "==") opc = Opcode::OP_EQ;
            else if (bin_expr->op == ">") opc = Opcode::OP_GT;
            else if (bin_expr->op == "<") opc = Opcode::OP_LT;
            else if (bin_expr->op == "&&") opc = Opcode::OP_AND;
            else if (bin_expr->op == "||") opc = Opcode::OP_OR;
            else if (bin_expr->op == "in") opc = Opcode::OP_IN;
            else assert(false && "gen_expr: 未支持的二元运算符");

            curr_code_list.emplace_back(
                opc, 
                std::vector<size_t>{},
                expr->start_ln, 
                expr->end_ln
            );
            curr_lineno_map.emplace_back(curr_code_list.size() - 1, expr->start_ln);
            break;
        }
        case AstType::UnaryExpr: {
            // 一元运算：生成操作数 -> 运算指令
            auto* unary_expr = static_cast<UnaryExpr*>(expr);
            gen_expr(unary_expr->operand.get());
                
            Opcode opc;
            if (unary_expr->op == "-") opc = Opcode::OP_NEG;
            else if (unary_expr->op == "!") opc = Opcode::OP_NOT;
            else assert(false && "gen_expr: 未支持的一元运算符");

            curr_code_list.emplace_back(
                opc, 
                std::vector<size_t>{},
                expr->start_ln, 
                expr->end_ln
            );
            curr_lineno_map.emplace_back(curr_code_list.size() - 1, expr->start_ln);
            break;
        }
        case AstType::CallExpr:
            gen_fn_call(static_cast<CallExpr*>(expr));
            break;
        case AstType::DictDeclExpr:
            gen_dict(static_cast<DictDeclExpr*>(expr));
            break;
        case AstType::ListExpr: {
            // 数组字面量：生成每个元素表达式 -> 构建数组（简化：用LOAD_CONST存储数组对象）
            auto* arr_expr = static_cast<ListExpr*>(expr);
            model::List* arr = new model::List();
            // 处理数组元素
            for (auto& elem : arr_expr->elements) {
                gen_expr(elem.get());
                // 弹出元素值存入数组（简化：假设栈顶为元素值）
                size_t elem_const_idx = curr_const.size() - 1;
                arr->val.emplace_back(curr_const[elem_const_idx]);
                curr_const[elem_const_idx]->make_ref();
            }
            // 加载数组对象
            size_t arr_const_idx = get_or_add_const(curr_const, arr);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST, 
                std::vector<size_t>{arr_const_idx},
                expr->start_ln, 
                expr->end_ln
            );
            break;
        }
        case AstType::GetMemberExpr: {
            // 获取成员：生成对象表达式 -> 加载属性名 -> GET_ATTR指令
            auto* get_mem = static_cast<GetMemberExpr*>(expr);
            gen_expr(get_mem->father.get()); // 生成对象IR
            size_t name_idx = get_or_add_name(curr_names, get_mem->child->name);
            curr_code_list.emplace_back(
                Opcode::GET_ATTR, 
                std::vector<size_t>{name_idx},
                expr->start_ln, 
                expr->end_ln
                );
            break;
        }
        case AstType::SetMemberExpr: {
            // 设置成员：生成对象表达式 -> 生成值表达式 -> 加载属性名 -> SET_ATTR指令
            auto* set_mem = static_cast<SetMemberExpr*>(expr);
            gen_expr(set_mem->g_mem.get()); // 生成对象IR（假设g_mem包含对象和属性名）
            gen_expr(set_mem->val.get());   // 生成值IR
            auto* get_mem = static_cast<GetMemberExpr*>(set_mem->g_mem.get());
            size_t name_idx = get_or_add_name(curr_names, get_mem->child->name);
            curr_code_list.emplace_back(
                Opcode::SET_ATTR, 
                std::vector<size_t>{name_idx},
                expr->start_ln, 
                expr->end_ln
            );
            break;
        }
        case AstType::LambdaDeclExpr: {
            // 匿名函数：同普通函数声明，生成函数对象后加载
            auto* lambda = static_cast<LambdaDeclExpr*>(expr);
            // 临时保存当前模块级代码容器
            auto save_code = curr_code_list;
            auto save_names = curr_names;
            auto save_const = curr_const;

            // 生成lambda函数体IR
            model::Function* lambda_fn = new model::Function();
            lambda_fn->name = lambda->name.empty() ? "<lambda>" : lambda->name;
            lambda_fn->argc = lambda->params.size();

            // 初始化lambda代码容器
            curr_code_list.clear();
            curr_names.clear();
            curr_const.clear();

            // 添加参数到lambda变量表
            for (const auto& param : lambda->params) {
                get_or_add_name(curr_names, param);
            }

            // 生成lambda函数体
            gen_block(lambda->body.get());
            // 确保lambda有返回值（无显式返回则返回Nil）
            if (curr_code_list.empty() || curr_code_list.back().opc != Opcode::RET) {
                model::Nil* nil = new model::Nil();
                size_t nil_idx = get_or_add_const(curr_const, nil);
                curr_code_list.emplace_back(
                    Opcode::LOAD_CONST, 
                    std::vector<size_t>{nil_idx},
                    lambda->body->start_ln, 
                    lambda->body->end_ln
                );
                curr_code_list.emplace_back(
                    Opcode::RET, 
                    std::vector<size_t>{},
                    lambda->body->end_ln, 
                    lambda->body->end_ln
                );
            }

            // 生成lambda代码对象
            lambda_fn->code = make_code_obj();
            lambda_fn->code->make_ref();

            // 恢复模块级代码容器
            curr_code_list = save_code;
            curr_names = save_names;
            curr_const = save_const;

            // 加载lambda函数对象
            size_t fn_const_idx = get_or_add_const(curr_const, lambda_fn);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST, 
                std::vector<size_t>{fn_const_idx},
                expr->start_ln, 
                expr->end_ln
            );
            break;
        }
        default:
            assert(false && "gen_expr: 未处理的表达式类型");
    }
}

void IRGenerator::gen_fn_call(CallExpr* call_expr) {
    assert(call_expr && "gen_fn_call: 函数调用节点为空");
    // 生成函数参数IR（参数按顺序压栈）
    for (auto& arg : call_expr->args) {
        gen_expr(arg.get());
    }

    // 生成函数对象IR（栈顶为函数对象）
    gen_expr(call_expr->callee.get());

    // 生成CALL指令（简化：操作数为参数个数）
    curr_code_list.emplace_back(
        Opcode::CALL, 
        std::vector<size_t>{call_expr->args.size()},
        call_expr->start_ln, 
        call_expr->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, call_expr->start_ln);
}

void IRGenerator::gen_dict(DictDeclExpr* dict_expr) {
    assert(dict_expr && "gen_dict: 字典节点为空");
    // 处理字典键值对（生成值表达式IR）
    model::Dictionary* dict = new model::Dictionary();
    for (auto& [key, val_expr] : dict_expr->init_list) {
        gen_expr(val_expr.get());
        // 弹出值存入字典（简化：假设栈顶为值，键为字符串常量）
        size_t val_const_idx = curr_const.size() - 1;
        model::Object* val = curr_const[val_const_idx];
        val->make_ref();
        
        // 键转换为String对象
        model::String* key_obj = new model::String();
        key_obj->val = key;
        key_obj->make_ref();

        // 存入字典（简化：假设Dictionary有键值对存储逻辑）
        dict->attrs[key] = val;
    }

    // 将字典对象加入常量池并加载
    size_t dict_const_idx = get_or_add_const(curr_const, dict);
    curr_code_list.emplace_back(
        Opcode::LOAD_CONST, 
        std::vector<size_t>{dict_const_idx},
        dict_expr->start_ln, 
        dict_expr->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, dict_expr->start_ln);
}

void IRGenerator::gen_fn_decl(FuncDefStmt* fn_decl) {
    assert(fn_decl && "gen_fn_decl: 函数声明节点为空");
    // 临时保存当前模块级代码容器（函数体为独立作用域）
    auto save_code = curr_code_list;
    auto save_names = curr_names;
    auto save_const = curr_const;
    auto save_lineno = curr_lineno_map;

    // 初始化函数级代码容器
    curr_code_list.clear();
    curr_names.clear();
    curr_const.clear();
    curr_lineno_map.clear();
    block_stack.empty();

    // 添加函数参数到变量表
    for (const auto& param : fn_decl->params) {
        get_or_add_name(curr_names, param);
    }

    // 生成函数体IR
    gen_block(fn_decl->body.get());

    // 确保函数有返回值（无显式return则返回Nil）
    if (curr_code_list.empty() || curr_code_list.back().opc != Opcode::RET) {
        model::Nil* nil = new model::Nil();
        size_t nil_idx = get_or_add_const(curr_const, nil);
        curr_code_list.emplace_back(
            Opcode::LOAD_CONST, 
            std::vector<size_t>{nil_idx},
            fn_decl->body->end_ln, 
            fn_decl->body->end_ln
        );
        curr_code_list.emplace_back(
            Opcode::RET, 
            std::vector<size_t>{},
            fn_decl->body->end_ln, 
            fn_decl->body->end_ln
        );
        curr_lineno_map.emplace_back(curr_code_list.size() - 1, fn_decl->body->end_ln);
    }

    // 生成函数代码对象
    model::CodeObject* fn_code = make_code_obj();
    assert(fn_code && "gen_fn_decl: 函数CodeObject创建失败");

    // 初始化函数对象
    model::Function* fn = new model::Function();
    fn->name = fn_decl->name;
    fn->argc = fn_decl->params.size();
    fn->code = fn_code;
    fn->code->make_ref();

    // 恢复模块级代码容器
    curr_code_list = save_code;
    curr_names = save_names;
    curr_const = save_const;
    curr_lineno_map = save_lineno;

    // 将函数对象加入模块常量池并存储为全局变量
    size_t fn_const_idx = get_or_add_const(curr_const, fn);
    size_t fn_name_idx = get_or_add_name(curr_names, fn_decl->name);
    
    // 加载函数对象 + 存储为全局变量
    curr_code_list.emplace_back(
        Opcode::LOAD_CONST, 
        std::vector<size_t>{fn_const_idx},
        fn_decl->start_ln, 
        fn_decl->start_ln
    );
    curr_code_list.emplace_back(
        Opcode::SET_GLOBAL, 
        std::vector<size_t>{fn_name_idx},
        fn_decl->start_ln, 
        fn_decl->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, fn_decl->end_ln);
}

void IRGenerator::gen_if(IfStmt* if_stmt) {
    assert(if_stmt && "gen_if: if节点为空");
    // 生成条件表达式IR
    gen_expr(if_stmt->condition.get());

    // 生成JUMP_IF_FALSE指令（目标先占位，后续填充）
    size_t jump_if_false_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP_IF_FALSE, 
        std::vector<size_t>{0}, // 占位目标索引
        if_stmt->condition->start_ln, 
        if_stmt->condition->end_ln
    );

    // 生成then块IR
    gen_block(if_stmt->thenBlock.get());

    // 生成JUMP指令（跳过else块，目标占位）
    size_t jump_else_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP, 
        std::vector<size_t>{0}, // 占位目标索引
        if_stmt->thenBlock->end_ln, 
        if_stmt->thenBlock->end_ln
    );

    // 填充JUMP_IF_FALSE的目标（else块开始位置）
    curr_code_list[jump_if_false_idx].opn_list[0] = curr_code_list.size();

    // 生成else块IR（存在则生成）
    if (if_stmt->elseBlock) {
        gen_block(if_stmt->elseBlock.get());
    }

    // 填充JUMP的目标（if-else结束位置）
    curr_code_list[jump_else_idx].opn_list[0] = curr_code_list.size();

    // 记录行号映射
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, if_stmt->end_ln);
}

void IRGenerator::gen_while(WhileStmt* while_stmt) {
    assert(while_stmt && "gen_while: while节点为空");
    // 记录循环入口位置（条件判断开始）
    size_t loop_entry_idx = curr_code_list.size();
    block_stack.push(loop_entry_idx); // 用于continue跳转

    // 生成循环条件IR
    gen_expr(while_stmt->condition.get());

    // 生成JUMP_IF_FALSE指令（目标：循环结束位置，占位）
    size_t jump_out_idx = curr_code_list.size();
    curr_code_list.emplace_back(
        Opcode::JUMP_IF_FALSE, 
        std::vector<size_t>{0},
        while_stmt->condition->start_ln, 
        while_stmt->condition->end_ln
    );

    // 记录循环体结束位置（用于break跳转）
    size_t loop_exit_idx = curr_code_list.size();
    block_stack.push(loop_exit_idx); // 用于break跳转

    // 生成循环体IR
    gen_block(while_stmt->body.get());

    // 生成JUMP指令（跳回循环入口）
    curr_code_list.emplace_back(
        Opcode::JUMP, 
        std::vector<size_t>{loop_entry_idx},
        while_stmt->body->end_ln, 
        while_stmt->body->end_ln
    );

    // 填充JUMP_IF_FALSE的目标（循环结束位置）
    curr_code_list[jump_out_idx].opn_list[0] = curr_code_list.size();

    // 弹出循环栈帧
    block_stack.pop();
    block_stack.pop();

    // 记录行号映射
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, while_stmt->end_ln);
}

void IRGenerator::gen_literal(Expression* literal_expr) {
    assert(literal_expr && "gen_literal: 字面量节点为空");
    model::Object* const_obj = nullptr;

    switch (literal_expr->ast_type) {
        case AstType::NumberExpr:
            const_obj = make_int_obj(static_cast<NumberExpr*>(literal_expr));
            break;
        case AstType::StringExpr:
            const_obj = make_string_obj(static_cast<StringExpr*>(literal_expr));
            break;
        default:
            assert(false && "gen_literal: 未处理的字面量类型");
    }

    // 生成LOAD_CONST指令（加载字面量常量）
    assert(const_obj && "gen_literal: 常量对象创建失败");
    size_t const_idx = get_or_add_const(curr_const, const_obj);
    curr_code_list.emplace_back(
        Opcode::LOAD_CONST, 
        std::vector<size_t>{const_idx},
        literal_expr->start_ln, 
        literal_expr->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, literal_expr->start_ln);
}

// -------------------------- Protected 成员函数实现 --------------------------
model::CodeObject* IRGenerator::make_code_obj() {
    model::CodeObject* code_obj = new model::CodeObject();
    // 复制当前代码指令集
    code_obj->code = curr_code_list;
    // 复制常量池（管理引用计数）
    for (auto* obj : curr_const) {
        obj->make_ref();
        code_obj->consts.emplace_back(obj);
    }
    // 复制变量名表
    code_obj->names = curr_names;
    // 复制行号映射
    code_obj->lineno_map = curr_lineno_map;
    return code_obj;
}

model::Int* IRGenerator::make_int_obj(NumberExpr* num_expr) {
    assert(num_expr && "make_int_obj: 数字节点为空");
    model::Int* int_obj = new model::Int();
    // 转换字符串值为BigInt（假设deps::BigInt支持字符串构造）
    int_obj->val = deps::BigInt(num_expr->value);
    return int_obj;
}

model::Dec* IRGenerator::make_dec_obj(NumberExpr* num_expr) {
    assert(num_expr && "make_dec_obj: 数字节点为空");
    model::Dec* dec_obj = new model::Dec();
    // 简化：假设Dec有double类型成员val，字符串转double
    dec_obj->val = std::stod(num_expr->value);
    return dec_obj;
}

model::Rational* IRGenerator::make_rational_obj(NumberExpr* num_expr) {
    assert(num_expr && "make_rational_obj: 数字节点为空");
    model::Rational* rational_obj = new model::Rational();
    // 简化：假设有理数为分数形式（如"3/4"），分割分子分母
    auto slash_pos = num_expr->value.find('/');
    if (slash_pos == std::string::npos) {
        // 整数形式（分母为1）
        rational_obj->numerator = deps::BigInt(num_expr->value);
        rational_obj->denominator = deps::BigInt(1);
    } else {
        std::string num_str = num_expr->value.substr(0, slash_pos);
        std::string den_str = num_expr->value.substr(slash_pos + 1);
        rational_obj->numerator = deps::BigInt(num_str);
        rational_obj->denominator = deps::BigInt(den_str);
        // 简化：不处理分母为0的情况（实际需断言）
        assert(rational_obj->denominator != deps::BigInt(0) && "make_rational_obj: 分母为0");
    }
    return rational_obj;
}

model::String* IRGenerator::make_string_obj(StringExpr* str_expr) {
    assert(str_expr && "make_string_obj: 字符串节点为空");
    model::String* str_obj = new model::String();
    str_obj->val = str_expr->value;
    return str_obj;
}

} // namespace kiz