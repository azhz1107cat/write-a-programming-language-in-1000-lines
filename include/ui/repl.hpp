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

namespace ui {

class Repl {
private:
    std::string prompt_;
    // 命令历史记录
    std::vector<std::string> cmd_history_;
    // 循环运行标志
    bool is_running_;

    /**
     * @brief 辅助函数：打印命令提示符
     * 确保提示符及时显示（刷新输出缓冲区），避免因缓冲导致显示延迟
     */
    void print_prompt() const {
        std::cout << prompt_;
        std::cout.flush(); // 强制刷新缓冲区
    }

    /**
     * @brief 辅助函数：添加命令到历史记录
     * 过滤空命令，避免存储无效输入
     * @param cmd 待添加的用户命令
     */
    void add_to_history(const std::string& cmd) {
        if (!cmd.empty()) {
            cmd_history_.emplace_back(cmd);
        }
    }

    /**
     * @brief 辅助函数：去除字符串首尾空白（空格、制表符等）
     * 处理用户输入中的无意空格，确保命令解析一致性
     * @param str 待处理的字符串
     * @return 去除首尾空白后的字符串
     */
    std::string trim(const std::string& str) const {
        auto left = std::find_if_not(str.begin(), str.end(), [](unsigned char c) {
            return std::isspace(c);
        });
        auto right = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
            return std::isspace(c);
        }).base();
        return (left < right) ? std::string(left, right) : std::string();
    }

public:
    /**
     * @brief 构造函数：初始化 REPL 配置
     * 默认提示符为 "> "，命令历史为空，初始设置循环为运行状态
     */
    Repl();

    /**
     * @brief 析构函数：默认空实现
     * 无动态分配资源（vector 自动管理），无需额外释放逻辑
     */
    ~Repl() = default;

    /**
     * @brief 读取用户输入
     * 流程：打印提示符 → 读取一行输入 → 去除首尾空白 → 添加到历史（非空时）
     * @return 处理后的用户输入字符串（空串表示用户输入仅空白）
     */
    std::string read();

    /**
     * @brief REPL 主循环
     * 持续执行“读取-解析-执行-打印”流程，直到 is_running_ 设为 false（如用户输入 exit）
     */
    void loop();

    /**
     * @brief 解析命令、执行并打印结果
     * 内置基础命令：
     * - exit：停止主循环，退出 REPL
     * - history：打印所有命令历史记录
     * - help：打印帮助信息（基础命令说明）
     * 自定义命令逻辑可在此函数内扩展
     * @param cmd 经 read() 处理后的用户命令
     */
    void eval_and_print(const std::string& cmd);

    /**
     * @brief 手动停止 REPL 循环
     * 外部可调用此接口强制终止主循环（如收到信号时）
     */
    void stop() {
        is_running_ = false;
    }

    /**
     * @brief 设置自定义命令提示符
     * @param prompt 新的提示符（如 "calc> "、"cmd# "）
     */
    void set_prompt(const std::string& prompt) {
        prompt_ = prompt;
    }

    /**
     * @brief 获取当前命令历史记录
     * @return 命令历史的常量引用（只读，避免外部修改）
     */
    const std::vector<std::string>& get_history() const {
        return cmd_history_;
    }
};




} // namespace ui