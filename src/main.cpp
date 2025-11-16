/**
 * @file main.cpp
 * @brief 程序入口文件
 *
 * 负责初始化程序环境、解析命令行参数、启动repl
 * @author azhz1107cat
 * @date 20250-25
 */

#include "bigint.hpp"
#include "ui/repl.hpp"
#ifdef _WIN32
#include <windows.h>
#include <winnls.h>
#endif

#include <iostream>

#include "kiz.hpp"
/* 提供命令行帮助信息函数 */
void show_help(const char* prog_name);

/* 命令行参数解析函数 */
void args_parser(int argc, char* argv[]);

/*
 * 主函数
 */
int main(const int argc, char* argv[]) {
    args_parser(argc, argv);
    return 0;
}

/* 提供命令行帮助信息函数 */
void show_help(const char* prog_name) {
    printf("%s [指令] [参数]\n", prog_name);
    printf("指令:\n");
    printf("  没有参数      启动 REPL\n");
    printf("  repl         启动 REPL\n");
    printf("  <路径>        执行位于指定路径的kiz文件\n");
    printf("  run <路径>    执行位于指定路径的kiz文件\n");
    printf("  version      展示版本号\n");
    printf("  help         展示帮助\n");
}

void enable_ansi_escape() {
#ifdef _WIN32
    // Windows 平台的颜色支持
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD outMode = 0;
        if (GetConsoleMode(hOut, &outMode)) {
            // 只有在成功获取控制台模式时才尝试设置
            if (!SetConsoleMode(hOut, outMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                // 设置失败只是警告，不影响程序运行
                std::cerr << "Warning: Console does not support ANSI color (STD_OUTPUT)" << std::endl;
            }
        } else {
            // 获取控制台模式失败（可能在测试环境或无控制台情况）
            // 静默失败，不输出错误信息，以免影响测试
        }
    }

    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
    if (hErr != INVALID_HANDLE_VALUE) {
        DWORD errMode = 0;
        if (GetConsoleMode(hErr, &errMode)) {
            if (!SetConsoleMode(hErr, errMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                std::cerr << "Warning: Console does not support ANSI color (STD_ERROR)" << std::endl;
            }
        }
        // 获取模式失败时静默处理
    }

    // UTF-8 编码设置
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

/**
 * @brief 解析命令行参数（argc/argv）
 * @param argc 命令行参数个数（来自main函数）
 * @param argv 命令行参数数组（来自main函数）
 * @return void
 */
void args_parser(const int argc, char* argv[]) {
    // 程序名称
    enable_ansi_escape();
    const char* prog_name = argv[0];

    // 无参数：默认启动REPL
    if (argc == 1) {
        ui::Repl repl;
        repl.loop();
        return;
    }

    // 1个参数 : 处理 version/repl/help/路径
    if (argc == 2) {
        const std::string cmd = argv[1];
        if (cmd == "version") {
            // 显示版本
            std::cout << "kiz version :" << KIZ_VERSION << std::endl;
        } else if (cmd == "repl") {
            // 显式启动REPL
            ui::Repl repl;
            repl.loop();
        } else if (cmd == "help") {
            // 显示帮助信息
            show_help(prog_name);
        } else {
            // ToDo: ...
        }
        return;
    }

    // 2个参数 : 仅处理 run <path>
    if (argc == 3) {
        const std::string cmd = argv[1];
        if (cmd == "run") {
            std::string path = argv[2];
            // ToDo: ...
        } else {
            // 无效命令
            std::cerr << "错误: 无效指令 " << cmd << "\n";
            show_help(prog_name);
        }
        return;
    }

    // 参数过多 : 提示错误并显示帮助
    std::cerr << "错误: 太多参数";
    show_help(prog_name);
}