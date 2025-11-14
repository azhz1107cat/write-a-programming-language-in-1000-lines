/**
 * @file error_reporter.cpp
 * @brief 错误报告器（Error Reporter）核心实现
 * kiz错误报告器
 * @author azhz1107cat
 */

#include "util/src_manager.hpp"

#include <iostream>
#include <string>
namespace util {

std::string generate_separator(const int col_start, const int col_end, const int line_end) {
    std::stringstream ss;
    ss << std::to_string(line_end).size(); // 对齐行号
    for (int i = 1; i < col_start; ++i) ss << " ";
    const int length = std::max(1, col_end - col_start + 1);
    for (int i = 0; i < length; ++i) ss << "^";
    return ss.str();
}

void error_reporter(
    const std::string& src_path,
    const int& src_line_start,
    const int& src_line_end,
    const int& src_col_start,
    const int& src_col_end
) {
    // 获取错误位置的代码
    std::string error_line = get_slice(src_path, src_line_start, src_line_end);
    if (error_line.empty()) {
        error_line = "[Can't slice the source file]";
    }

    // // 格式化错误信息
    // std::cerr << "\n" << ConClr::RESET << ConClr::RED << "ERROR [" << error.err_code << "]: "
    //           << error.name << ConClr::RESET << "\n";
    // std::cerr << "  file: " << src_path << "\n";
    // std::cerr << "  pos: ln " << src_line_start;
    // if (src_line_start != src_line_end) {
    //     std::cerr << "-" << src_line_end;
    // }
    // std::cerr << ", ln " << src_col_start << "-" << src_col_end << "\n";
    // std::cerr << "  [Info]: " << error.content << "\n\n";
    //
    // // 显示错误行代码
    // std::cerr << "    " << src_line_start << " | " << error_line << "\n";
    // std::cerr << "      " << generate_separator(src_col_start, src_col_end, src_line_end) << "\n\n";
    //
    // std::exit(error.err_code);
}

} // namespace util