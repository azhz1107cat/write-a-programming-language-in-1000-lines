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

#include <cstdio>
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

/**
 * @brief 解析命令行参数（argc/argv）
 * @param argc 命令行参数个数（来自main函数）
 * @param argv 命令行参数数组（来自main函数）
 * @return void
 */
void args_parser(const int argc, char* argv[]) {
    // 程序名称
    const char* prog_name = argv[0];

    // 无参数：默认启动REPL
    if (argc == 1) {
        ui::Repl repl;
        return;
    }

    // 1个参数 : 处理 version/repl/help/路径
    if (argc == 2) {
        const std::string cmd = argv[1];
        if (cmd == "version") {
            // 显示版本
            printf("kiz version %s\n", "KIZ_VERSION");
        } else if (cmd == "repl") {
            // 显式启动REPL
          ui::Repl repl;
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