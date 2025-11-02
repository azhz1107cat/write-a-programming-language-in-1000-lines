/**
 * @file repl.cpp
 * @brief 交互式终端（Read Evaluate Print Loop，简称 REPL）核心实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */


ui::Repl::Repl() {
    // ToDo: ...
}

std::string ui::Repl::read() {
    std::string result;
    std::cout << ">>>";
    std::geine(std::cout, result);
    return result;
}

void ui::Repl::loop() {
    // ToDo: ...
}

void ui::Repl::eval_and_print() {
    
}

void ui::Repl::~Repl() {
    // ToDo: ...
}