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
#include "project_debugger.hpp"

namespace ui {

std::string Repl::read(const std::string& prompt) {
    std::string result;
    std::cout << prompt;
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
    std::string file_path = "<shell#>";
    kiz::Lexer lexer(file_path);
    kiz::Parser parser(file_path);
    kiz::IRGenerator ir_gen(file_path);
    kiz::Vm vm(file_path);

    auto tokens = lexer.tokenize(cmd);
    auto ast = parser.parse(tokens);
    auto ir = ir_gen.gen(std::move(ast));
    auto [stack_top, locals] = vm.load(ir);
    std::cout << stack_top->to_string() << std::endl;
}

} // namespace ui