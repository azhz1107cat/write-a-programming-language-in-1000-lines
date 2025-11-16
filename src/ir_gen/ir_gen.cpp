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
#include <algorithm>
#include <cassert>

#include "kiz.hpp"

namespace kiz {

size_t IRGenerator::get_or_add_name(std::vector<std::string>& names, const std::string& name) {
    auto it = std::find(names.begin(), names.end(), name);
    if (it != names.end()) {
        return std::distance(names.begin(), it);
    }
    names.emplace_back(name);
    return names.size() - 1;
}

// 辅助函数：获取常量在curr_const中的索引（不存在则添加）
size_t IRGenerator::get_or_add_const(std::vector<model::Object*>& consts, model::Object* obj) {
    const auto it = std::find(consts.begin(), consts.end(), obj);
    if (it != consts.end()) {
        return std::distance(consts.begin(), it);
    }
    obj->make_ref(); // 管理引用计数
    consts.emplace_back(obj);
    return consts.size() - 1;
}

model::Module* IRGenerator::gen(std::unique_ptr<BlockStmt> ast_into) {
    ast = std::move(ast_into);
    DEBUG_OUTPUT("generating...");
    // 检查AST根节点有效性（默认模块根为BlockStmt）
    assert(ast && ast->ast_type == AstType::BlockStmt && "gen: AST根节点非BlockStmt");
    const auto* root_block = ast.get();

    // 初始化模块级代码容器
    curr_code_list.clear();
    curr_names.clear();
    curr_consts.clear();
    curr_lineno_map.clear();

    // 处理模块顶层节点
    gen_block(root_block);

    return gen_mod(file_path,
        curr_names,
        curr_code_list,
        curr_consts,
        curr_lineno_map
    );
}

model::CodeObject* IRGenerator::make_code_obj() const {
    DEBUG_OUTPUT("making code object...");
    // 复制常量池（管理引用计数）
    std::vector<model::Object*> consts;
    for (auto* obj : curr_consts) {
        obj->make_ref();
        consts.emplace_back(obj);
    }
    const auto code_obj = new model::CodeObject(
        curr_code_list, consts, curr_names, curr_lineno_map
    );
    return code_obj;
}

model::Int* IRGenerator::make_int_obj(const NumberExpr* num_expr) {
    DEBUG_OUTPUT("making int object...");
    assert(num_expr && "make_int_obj: 数字节点为空");
    auto int_obj = new model::Int( deps::BigInt(num_expr->value) );
    return int_obj;
}


model::Rational* IRGenerator::make_rational_obj(NumberExpr* num_expr) {
    DEBUG_OUTPUT("making rational object...");
    assert(num_expr && "make_rational_obj: 数字节点为空");
    deps::Rational rational;

    // 简化：假设有理数为分数形式（如"3/4"），分割分子分母
    const auto slash_pos = num_expr->value.find('/');
    if (slash_pos == std::string::npos) {
        // 整数形式（分母为1）
        rational.numerator = deps::BigInt(num_expr->value);
        rational.denominator = deps::BigInt(1);
    } else {
        std::string num_str = num_expr->value.substr(0, slash_pos);
        std::string den_str = num_expr->value.substr(slash_pos + 1);
        rational.numerator = deps::BigInt(num_str);
        rational.denominator = deps::BigInt(den_str);
        // 简化：不处理分母为0的情况（实际需断言）
        assert(rational.denominator != deps::BigInt(0) && "make_rational_obj: 分母为0");
    }
    auto rational_obj = new model::Rational(rational);
    return rational_obj;
}

model::String* IRGenerator::make_string_obj(const StringExpr* str_expr) {
    DEBUG_OUTPUT("making string object...");
    assert(str_expr && "make_string_obj: 字符串节点为空");
    auto str_obj = new model::String(str_expr->value);
    return str_obj;
}

} // namespace kiz