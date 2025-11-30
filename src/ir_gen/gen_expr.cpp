#include "opcode.hpp"
#include "../../include/ir_gen.hpp"
#include "../../include/ast.hpp"
#include "../../include/models.hpp"

namespace kiz {

void IRGenerator::gen_expr(Expression* expr) {
    assert(expr && "gen_expr: 表达式节点为空");
    switch (expr->ast_type) {
        case AstType::NumberExpr:
            gen_literal(dynamic_cast<NumberExpr*>(expr));
            break;
        case AstType::StringExpr:
            gen_literal(dynamic_cast<StringExpr*>(expr));
            break;
        case AstType::IdentifierExpr: {
            // 标识符：生成LOAD_VAR指令（加载变量值）
            const auto* ident = dynamic_cast<IdentifierExpr*>(expr);
            const size_t name_idx = get_or_add_name(curr_names, ident->name);
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
            const auto* bin_expr = dynamic_cast<BinaryExpr*>(expr);
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
            else if (bin_expr->op == "and") opc = Opcode::OP_AND;
            else if (bin_expr->op == "or") opc = Opcode::OP_OR;
            else if (bin_expr->op == "not") opc = Opcode::OP_NOT;
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
            auto* unary_expr = dynamic_cast<UnaryExpr*>(expr);
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
            DEBUG_OUTPUT("gen fn call...");
            gen_fn_call(dynamic_cast<CallExpr*>(expr));
            break;
        case AstType::DictDeclExpr:
            gen_dict(dynamic_cast<DictDeclExpr*>(expr));
            break;
        case AstType::ListExpr: {
            auto list_expr = dynamic_cast<ListExpr*>(expr);
            for (const auto e: list_expr->elements) {
                gen_expr(e);
            }
            // 生成 OP_MAKE_LIST 指令
            curr_code_list.emplace_back(
                Opcode::MAKE_LIST,
                std::vector{list_expr->elements.size()},
                expr->start_ln,
                expr->end_ln
           );
            break;
        }
        case AstType::GetMemberExpr: {
            // 获取成员：生成对象表达式 -> 加载属性名 -> GET_ATTR指令
            auto* get_mem = dynamic_cast<GetMemberExpr*>(expr);
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
            const auto* set_mem = dynamic_cast<SetMemberExpr*>(expr);
            gen_expr(set_mem->g_mem.get()); // 生成对象IR（假设g_mem包含对象和属性名）
            gen_expr(set_mem->val.get());   // 生成值IR
            const auto* get_mem = dynamic_cast<GetMemberExpr*>(set_mem->g_mem.get());
            size_t name_idx = get_or_add_name(curr_names, get_mem->child->name);
            curr_code_list.emplace_back(
                Opcode::SET_ATTR,
                std::vector<size_t>{name_idx},
                expr->start_ln,
                expr->end_ln
            );
            break;
        }
        case AstType::FuncDeclExpr: {
            // 匿名函数：同普通函数声明，生成函数对象后加载
            auto* lambda = dynamic_cast<FnDeclExpr*>(expr);
            // 临时保存当前模块级代码容器
            auto save_code = curr_code_list;
            auto save_names = curr_names;
            auto save_const = curr_consts;

            // 初始化lambda代码容器
            curr_code_list.clear();
            curr_names.clear();
            curr_consts.clear();

            // 添加参数到lambda变量表
            for (const auto& param : lambda->params) {
                get_or_add_name(curr_names, param);
            }
            // 生成lambda函数体
            gen_block(lambda->body.get());
            // 确保lambda有返回值（无显式返回则返回Nil）
            if (curr_code_list.empty() || curr_code_list.back().opc != Opcode::RET) {
                const auto nil = new model::Nil();
                const size_t nil_idx = get_or_add_const(curr_consts, nil);
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

            const auto code_obj = new model::CodeObject(
                curr_code_list,
                curr_consts,
                curr_names,
                curr_lineno_map
            );

            // 生成lambda函数体IR
            const auto lambda_fn = new model::Function(
                lambda->name.empty() ? "<lambda>" : lambda->name,
                code_obj,
                lambda->params.size()
            );

            // 恢复模块级代码容器
            curr_code_list = save_code;
            curr_names = save_names;
            curr_consts = save_const;

            // 加载lambda函数对象
            const size_t fn_const_idx = get_or_add_const(curr_consts, lambda_fn);
            curr_code_list.emplace_back(
                Opcode::LOAD_CONST,
                std::vector{fn_const_idx},
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
    size_t arg_count = call_expr->args.size();

    // 生成所有参数的IR
    for (auto& arg : call_expr->args) {
        gen_expr(arg.get());
    }

    // 生成 OP_MAKE_LIST 指令：将栈顶 arg_count 个元素打包成 List，压回栈
    curr_code_list.emplace_back(
        Opcode::MAKE_LIST,
        std::vector<size_t>{arg_count},  // 操作数：需要打包的元素个数
        call_expr->start_ln,
        call_expr->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, call_expr->start_ln);

    // 生成函数对象的IR（压到栈顶）
    gen_expr(call_expr->callee.get());

    // 生成 CALL 指令（操作数保留参数个数，用于 Function 校验参数数量）
    curr_code_list.emplace_back(
        Opcode::CALL,
        std::vector<size_t>{arg_count},  // 仍传参数个数（Function 需校验 List 元素个数）
        call_expr->start_ln,
        call_expr->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, call_expr->start_ln);
}

void IRGenerator::gen_dict(DictDeclExpr* expr) {
    assert(expr && "gen_dict: 字典节点为空");
    // 处理字典键值对（生成值表达式IR）
    auto dict = new model::Dictionary();
    for (auto& [key, val_expr] : expr->init_list) {
        gen_expr(val_expr.get());
        // 弹出值存入字典（简化：假设栈顶为值，键为字符串常量）
        size_t val_const_idx = curr_consts.size() - 1;
        model::Object* val = curr_consts[val_const_idx];
        val->make_ref();

        // 键转换为String对象
        auto key_obj = new model::String(key);
        key_obj->make_ref();

        // 存入字典
        dict->attrs.insert(key, val);
    }

    // 将字典对象加入常量池并加载
    size_t dict_const_idx = get_or_add_const(curr_consts, dict);
    curr_code_list.emplace_back(
        Opcode::LOAD_CONST,
        std::vector<size_t>{dict_const_idx},
        expr->start_ln,
        expr->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, expr->start_ln);
}

void IRGenerator::gen_literal(Expression* expr) {
    assert(expr && "gen_literal: 字面量节点为空");
    model::Object* const_obj = nullptr;

    switch (expr->ast_type) {
    case AstType::NumberExpr:
        const_obj = make_int_obj(dynamic_cast<NumberExpr*>(expr));
        break;
    case AstType::StringExpr:
        const_obj = make_string_obj(dynamic_cast<StringExpr*>(expr));
        break;
    default:
        assert(false && "gen_literal: 未处理的字面量类型");
    }

    // 生成LOAD_CONST指令（加载字面量常量）
    assert(const_obj && "gen_literal: 常量对象创建失败");
    size_t const_idx = get_or_add_const(curr_consts, const_obj);
    curr_code_list.emplace_back(
        Opcode::LOAD_CONST,
        std::vector<size_t>{const_idx},
        expr->start_ln,
        expr->end_ln
    );
    curr_lineno_map.emplace_back(curr_code_list.size() - 1, expr->start_ln);
}

}
