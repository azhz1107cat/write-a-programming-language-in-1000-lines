/**
 * @file repl.cpp
 * @brief 交互式终端（Read Evaluate Print Loop，简称 REPL）核心实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

namespace ui {

Repl::Repl() {
    // ToDo: ...
}

std::string Repl::read(std::string prompt) {
    std::string result;
    std::cout << prompt;
    std::geine(std::cout, result);
    return trim(result);
}

void Repl::loop() {
    while (true) {
        auto code = read(">>>");
        eval_and_print(code);
    }
}

void Repl::eval_and_print(std::string code) {
    kiz::Lexer lexer;
    kiz::Parser parser;
    kiz::IRGenerator ir_gen;
    kiz::Vm vm;

    auto tokens = lexer.tokenize(code);
    auto ast = parser.parse(tokens);
    auto ir = ir_gen.gen(std::move(ast));
    auto result = vm.load(ir);
    std::cout << model::to_string(result.stack_top) << std::endl;
}

void Repl::~Repl() {
    // ToDo: ...
}

} // namespace ui