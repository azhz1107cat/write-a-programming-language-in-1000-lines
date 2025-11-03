/**
 * @file opcode.hpp
 * @brief 虚拟机指令集(VM Opcode)核心定义
 * @author azhz1107cat
 * @date 2025-10-25
 */

namespace kiz {

enum class Opcode {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV
    OP_MOD, OP_POW, OP_NEG
    OP_EQ, OP_GT, OP_LT,
    OP_AND, OP_NOT, OP_OR,
    OP_IS, OP_IN,
    CALL, RET,
    GET_ATTR, SET_ATTR,
    LOAD_VAR, LOAD_CONST,
    SET_GLOBAL, SET_LOCAL, SET_NONLOCAL,
    JUMP, JUMP_IF_FALSE, THROW, 
    MAKE_LIST, MAKE_TUPLE, MAKE_DICT,
    POP_TOP, SWAP, COPY_TOP
};

} // namespace kiz