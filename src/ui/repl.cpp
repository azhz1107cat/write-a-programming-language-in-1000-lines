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
    auto tokens = lexer_.tokenize(cmd);
    auto ast = parser_.parse(tokens);
    auto ir = ir_gen_.gen(std::move(ast));
    auto [stack_top, locals] = vm_.load(ir);
    if (stack_top != nullptr) {
        if (not dynamic_cast<model::Nil*>(stack_top)) {
            std::cout << stack_top->to_string() << std::endl;
        }
        stack_top->del_ref();
    }
}

} // namespace ui