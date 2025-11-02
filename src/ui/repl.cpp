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
    return result;
}

void Repl::loop() {
    // ToDo: ...
}

void Repl::eval_and_print(std::string code) {
    Lexer lexer;
    Parser parser;
    IRGenerator ir_gen;
    Vm vm;

    auto tokens = lexer.tokenize(code);
    auto ast = parser.parse(tokens);
    auto ir = ir_gen.gen(std::move(ast));
    auto result = vm.load(ir);
    std::cout << result.stack_top << std::endl;
}

void Repl::~Repl() {
    // ToDo: ...
}

} // namespace ui