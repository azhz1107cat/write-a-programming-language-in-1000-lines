/**
 * @file main.cpp
 * @brief 程序入口文件
 * 
 * 负责初始化程序环境、解析命令行参数、启动repl
 * @author azhz1107cat
 * @date 20250-25
 */

/**
 * 命令行参数解析函数
 */
void args_parser(int argc, char* argv[]);

/**
 * 主函数
 */
int main(int argc, char* argv[]) {
    args_parser(argc, argv);
    return 0;
}

/**
 * @brief 解析命令行参数（argc/argv）
 * @param argc 命令行参数个数（来自main函数）
 * @param argv 命令行参数数组（来自main函数）
 * @param prog_name 程序名称（argv[0]，用于打印帮助/错误信息）
 * @return void
 */
void args_parser(int argc, char* argv[]){
    return;
}