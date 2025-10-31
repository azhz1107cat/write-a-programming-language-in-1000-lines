/**
 * @file vm.cpp
 * @brief 虚拟机（IR Generator）核心实现
 * 用于运行IR
 * @author azhz1107cat
 * @date 2025-10-25
 */

void kiz::Vm::load(model::Module* src_module) {

}

VmState kiz::Vm::exec(kiz::Introduction introduction) {
    switch (introduction.opc) {
        case Opcode::OP_ADD:
            break;

        case Opcode::OP_SUB:
            break;

        case Opcode::OP_MUL:
            break;

        case Opcode::OP_DIV:
            break;

        case Opcode::OP_MOD:
            break;

        case Opcode::OP_POW:
            break;

        case Opcode::OP_NEG:
            break;

        case Opcode::OP_EQ:
            break;

        case Opcode::OP_GT:
            break;

        case Opcode::OP_LT:
            break;

        case Opcode::OP_AND:
            break;

        case Opcode::OP_NOT:
            break;

        case Opcode::OP_OR:
            break;

        case Opcode::OP_IS:
            break;

        case Opcode::OP_IN:
            break;

        case Opcode::CALL:
            break;

        case Opcode::RET:
            break;

        case Opcode::GET_ATTR:
            break;
        case Opcode::SET_ATTR:
            break;

        case Opcode::LOAD_VAR:
            break;

        case Opcode::LOAD_CONST:
            break;

        case Opcode::SET_GLOBAL:
            break;

        case Opcode::SET_LOCAL:
            break;

        case Opcode::SET_NONLOCAL:
            break;

        case Opcode::JUMP:
            break;

        case Opcode::JUMP_IF_FALSE:
            break;

        case Opcode::POP_TOP:
            break;

        case Opcode::SWAP:
            break;

        case Opcode::COPY_TOP:
            break;

        default:
            break;
    }
}

void kiz::Vm::eval_call() {

}

void kiz::Vm::eval_set_member() {

}

void kiz::Vm::eval_ret() {

}