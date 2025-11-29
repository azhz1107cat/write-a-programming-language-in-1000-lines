/**
 * @file error_reporter.cpp
 * @brief 错误报告器（Error Reporter）核心实现
 * kiz错误报告器
 * @author azhz1107cat
 */

#include "util/error_reporter.hpp"

#include "util/src_manager.hpp"

#include <iostream>
#include <string>

#include "repl/color.hpp"

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
    const int& src_col_end,
    const ErrorInfo& error
) {
    // 获取错误行代码
    std::string error_line = get_slice(src_path, src_line_start, src_line_end);
    if (error_line.empty()) {
        error_line = "[Can't slice the source file]";
    }

    // 计算箭头位置：行号前缀长度 + 列偏移（列从1开始）
    const std::string line_prefix = std::to_string(src_line_start) + " | ";
    const size_t caret_offset = line_prefix.size() + (src_col_start - 1);
    // 生成箭头（单个^或连续^，匹配错误列范围）
    const std::string caret = std::string(src_col_end - src_col_start + 1, '^');

    // 格式化输出（颜色高亮+固定格式）
    std::cout << std::endl;
    // 文件路径
    std::cout << Color::BRIGHT_BLUE << "File \"" << src_path << "\"" << Color::RESET << std::endl;
    // 行号 + 错误代码行
    std::cout << Color::WHITE << line_prefix << error_line << Color::RESET << std::endl;
    // 箭头（对准错误列）
    std::cout << std::string(caret_offset, ' ') << Color::BRIGHT_RED << caret << Color::RESET << std::endl;
    // 错误信息（类型加粗红 + 内容白）
    std::cout << Color::BOLD << Color::BRIGHT_RED << error.name
              << Color::RESET << Color::WHITE << " : " << error.content
              << Color::RESET << std::endl;
    std::cout << std::endl;

    // 退出程序（携带错误码）
    std::exit(error.err_code);
}

} // namespace util