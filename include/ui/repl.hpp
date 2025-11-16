/**
 * @file repl.hpp
 * @brief 交互式终端（Read Evaluate Print Loop，简称 REPL）核心实现
 * 提供基础的“读取-解析-执行-打印”循环，支持命令历史、退出指令等基础功能，
 * 可扩展自定义命令解析逻辑，适配不同场景（如表达式计算、工具交互等）。
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <algorithm>

#include "ir_gen.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace ui {

class Repl {
    std::string prompt_;
    std::vector<std::string> cmd_history_;
    bool is_running_;

    kiz::Vm vm_;

    void add_to_history(const std::string& cmd) {
        if (!cmd.empty()) {
            cmd_history_.emplace_back(cmd);
        }
    }

    [[nodiscard]] static std::string trim(const std::string& str) {
        const auto left = std::find_if_not(str.begin(), str.end(), [](unsigned char c) {
            return std::isspace(c);
        });
        auto right = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
            return std::isspace(c);
        }).base();
        return (left < right) ? std::string(left, right) : std::string();
    }

public:
    Repl() {
        is_running_ = true;
        
        kiz::Vm vm_("<shell#>");
    }
    ~Repl() = default;

    static std::string read(const std::string& prompt);
    void loop();

    static void eval_and_print(const std::string& cmd);

    void stop() { is_running_ = false; }

    [[nodiscard]] const std::vector<std::string>& get_history() const {
        return cmd_history_;
    }
};




} // namespace ui