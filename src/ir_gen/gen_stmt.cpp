#include "opcode.hpp"
#include "../../include/ir_gen.hpp"
#include "../../include/ast.hpp"
#include "../../include/models.hpp"

namespace kiz {

model::Module* IRGenerator::gen_mod(
    const std::string& module_name,
    const std::vector<std::string>& names,
    const std::vector<Instruction>& code_list,
    const std::vector<model::Object*>& consts,
    const std::vector<std::tuple<size_t, size_t>>& lineno_map
)
{
    const auto code_obj = new model::CodeObject(
        code_list,
        consts,
        names,
        lineno_map
    );
    const auto module_obj = new model::Module(
        module_name,
        code_obj
    );
    return module_obj;
}


// 处理代码块（生成块内所有语句的IR）
void IRGenerator::gen_block(const BlockStmt* block) {
    if (!block) return;
    for (auto& stmt : block->statements) {
        switch (stmt->ast_type) {
            case AstType::AssignStmt: {
                // 变量声明：生成初始化表达式IR + 存储变量指令
                const auto* var_decl = dynamic_cast<AssignStmt*>(stmt.get());
                gen_expr(var_decl->expr.get()); // 生成初始化表达式IR
                const size_t name_idx = get_or_add_name(curr_names, var_decl->name);

                curr_code_list.emplace_back(
                    Opcode::SET_LOCAL,
                    std::vector<size_t>{name_idx},
                    stmt->start_ln,
                    stmt->end_ln
                );
                break;
            }
            case AstType::NonlocalAssignStmt: {
                // 变量声明：生成初始化表达式IR + 存储变量指令
                const auto* var_decl = dynamic_cast<AssignStmt*>(stmt.get());
                gen_expr(var_decl->expr.get()); // 生成初始化表达式IR
                const size_t name_idx = get_or_add_name(curr_names, var_decl->name);

                curr_code_list.emplace_back(
                    Opcode::SET_NONLOCAL,
                    std::vector<size_t>{name_idx},
                    stmt->start_ln,
                    stmt->end_ln
                );
                break;
            }

            case AstType::GlobalAssignStmt: {
                // 变量声明：生成初始化表达式IR + 存储变量指令
                const auto* var_decl = dynamic_cast<AssignStmt*>(stmt.get());
                gen_expr(var_decl->expr.get()); // 生成初始化表达式IR
                const size_t name_idx = get_or_add_name(curr_names, var_decl->name);

                curr_code_list.emplace_back(
                    Opcode::SET_GLOBAL,
                    std::vector<size_t>{name_idx},
                    stmt->start_ln,
                    stmt->end_ln
                );
                break;
            }
            case AstType::ExprStmt: {
                // 表达式语句：生成表达式IR + 弹出结果（避免栈泄漏）
                auto* expr_stmt = dynamic_cast<ExprStmt*>(stmt.get());
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
                gen_if(dynamic_cast<IfStmt*>(stmt.get()));
                break;
            case AstType::WhileStmt:
                gen_while(dynamic_cast<WhileStmt*>(stmt.get()));
                break;
            case AstType::ReturnStmt: {
                // 返回语句：生成返回值表达式IR + RET指令
                auto* ret_stmt = dynamic_cast<ReturnStmt*>(stmt.get());
                if (ret_stmt->expr) {
                    gen_expr(ret_stmt->expr.get());
                } else {
                    // 无返回值时压入Nil常量
                    auto* nil = new model::Nil();
                    const size_t const_idx = get_or_add_const(curr_consts, nil);
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
                break;
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
            case AstType::NextStmt:
                // Continue语句：跳转到循环条件位置（依赖block_stack记录循环入口）
                assert(block_stack.size() >= 2 && "NextStmt: 无活跃循环块");
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

void IRGenerator::gen_fn_decl(FnDeclExpr* fn_decl) {
    assert(fn_decl && "gen_fn_decl: 函数声明节点为空");
    // 临时保存当前模块级代码容器（函数体为独立作用域）
    auto save_code = curr_code_list;
    auto save_names = curr_names;
    auto save_const = curr_consts;
    auto save_lineno = curr_lineno_map;

    // 初始化函数级代码容器
    curr_code_list.clear();
    curr_names.clear();
    curr_consts.clear();
    curr_lineno_map.clear();

    // 添加函数参数到变量表
    for (const auto& param : fn_decl->params) {
        get_or_add_name(curr_names, param);
    }

    // 生成函数体IR
    gen_block(fn_decl->body.get());

    // 确保函数有返回值（无显式return则返回Nil）
    if (curr_code_list.empty() || curr_code_list.back().opc != Opcode::RET) {
        const auto nil = new model::Nil();
        const size_t nil_idx = get_or_add_const(curr_consts, nil);
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
    const auto fn = new model::Function(
        fn_decl->name, fn_code, fn_decl->params.size()
    );

    // 恢复模块级代码容器
    curr_code_list = save_code;
    curr_names = save_names;
    curr_consts = save_const;
    curr_lineno_map = save_lineno;

    // 将函数对象加入模块常量池并存储为全局变量
    size_t fn_const_idx = get_or_add_const(curr_consts, fn);
    size_t fn_name_idx = get_or_add_name(curr_names, fn_decl->name);

    // 加载函数对象 + 存储为全局变量
    curr_code_list.emplace_back(
        Opcode::LOAD_CONST,
        std::vector<size_t>{fn_const_idx},
        fn_decl->start_ln,
        fn_decl->start_ln
    );
    curr_code_list.emplace_back(
        Opcode::SET_LOCAL,
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
    const size_t jump_out_idx = curr_code_list.size();
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

}
