/**
 * @file error_reporter.hpp
 * @brief 错误报告器（Error Reporter）核心定义
 * kiz错误报告器
 * @author azhz1107cat
 */

#pragma once


namespace util {

struct ErrorInfo {
    const std::string name;
    const std::string content;
    int err_code;
};

std::string generate_separator(const int col_start, const int col_end, const int line_end);

void error_reporter(
    const std::string& src_path,
    const int& src_line_start,
    const int& src_line_end,
    const int& src_col_start,
    const int& src_col_end,
    const ErrorInfo& error
);

}// namespace util