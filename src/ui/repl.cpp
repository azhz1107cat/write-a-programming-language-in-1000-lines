/**
 * @file repl.cpp
 * @brief 交互式终端（Read Evaluate Print Loop，简称 REPL）核心实现
 *
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include "ui/repl.hpp"

#include "lexer.hpp"
#include "vm.hpp"
#include "ir_gen.hpp"
#include "parser.hpp"
#include "kiz.hpp"
#include "ui/color.hpp"

namespace ui {

std::string Repl::read(const std::string& prompt) {
    std::string result;
    std::cout << Color::BRIGHT_MAGENTA << prompt << Color::RESET;
    std::cout.flush();
    std::getline(std::cin, result);
    return trim(result);
}

void Repl::loop() {
    DEBUG_OUTPUT("start repl loop");
    while (is_running_) {
        auto code = read(">>>");
        add_to_history(code);
        eval_and_print(code);
    }
}

void Repl::eval_and_print(const std::string& cmd) {
    DEBUG_OUTPUT("repl eval_and_print...");
    const std::string file_path = "<shell#>";
    bool should_print = false;
    kiz::Lexer lexer(file_path);
    kiz::Parser parser(file_path);
    kiz::IRGenerator ir_gen(file_path);

    const auto tokens = lexer.tokenize(cmd);
    auto ast = parser.parse(tokens);
    if (ast->statements.size() > 0 and
        dynamic_cast<kiz::ExprStmt*>(ast->statements.back().get())
    )   should_print = true;

    const auto ir = ir_gen.gen(std::move(ast));
    if (cmd_history_.size() < 2) {
        vm_.load(ir);
    } else {
        assert(ir->code != nullptr && "No ir for run" );
        vm_.extend_code(ir->code);
    }

    DEBUG_OUTPUT("repl print");
    auto [stack_top, locals] = vm_.get_vm_state();
    if (stack_top != nullptr) {
        if (not dynamic_cast<model::Nil*>(stack_top) and should_print) {
            std::cout << stack_top->to_string() << std::endl;
        }
    }
}

} // namespace ui