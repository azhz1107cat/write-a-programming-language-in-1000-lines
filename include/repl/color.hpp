#pragma once
#include <string>

namespace Color {
    // 基础控制码：重置所有属性
    inline std::string RESET = "\033[0m";

    // 文本样式控制
    inline std::string BOLD = "\033[1m";        // 加粗
    inline std::string FAINT = "\033[2m";       // 淡色
    inline std::string ITALIC = "\033[3m";      // 斜体（部分终端支持）
    inline std::string UNDERLINE = "\033[4m";   // 下划线
    inline std::string BLINK = "\033[5m";       // 闪烁（部分终端支持）
    inline std::string REVERSE = "\033[7m";     // 反色显示
    inline std::string HIDDEN = "\033[8m";      // 隐藏文本（部分终端支持）

    // 前景色（文字颜色）- 标准色
    inline std::string BLACK = "\033[30m";      // 黑色
    inline std::string RED = "\033[31m";        // 红色
    inline std::string GREEN = "\033[32m";      // 绿色
    inline std::string YELLOW = "\033[33m";     // 黄色
    inline std::string BLUE = "\033[34m";       // 蓝色
    inline std::string MAGENTA = "\033[35m";    // 品红色
    inline std::string CYAN = "\033[36m";       // 青色
    inline std::string WHITE = "\033[37m";      // 白色

    // 前景色 - 高亮色（亮色系）
    inline std::string BRIGHT_BLACK = "\033[90m";   // 亮黑色（灰色）
    inline std::string BRIGHT_RED = "\033[91m";     // 亮红色
    inline std::string BRIGHT_GREEN = "\033[92m";   // 亮绿色
    inline std::string BRIGHT_YELLOW = "\033[93m";  // 亮黄色
    inline std::string BRIGHT_BLUE = "\033[94m";    // 亮蓝色
    inline std::string BRIGHT_MAGENTA = "\033[95m"; // 亮品红色
    inline std::string BRIGHT_CYAN = "\033[96m";    // 亮青色
    inline std::string BRIGHT_WHITE = "\033[97m";   // 亮白色

    // 背景色 - 标准色
    inline std::string BG_BLACK = "\033[40m";       // 背景黑色
    inline std::string BG_RED = "\033[41m";         // 背景红色
    inline std::string BG_GREEN = "\033[42m";       // 背景绿色
    inline std::string BG_YELLOW = "\033[43m";      // 背景黄色
    inline std::string BG_BLUE = "\033[44m";        // 背景蓝色
    inline std::string BG_MAGENTA = "\033[45m";     // 背景品红色
    inline std::string BG_CYAN = "\033[46m";        // 背景青色
    inline std::string BG_WHITE = "\033[47m";       // 背景白色

    // 背景色 - 高亮色（亮色系）
    inline std::string BG_BRIGHT_BLACK = "\033[100m";  // 背景亮黑色（灰色）
    inline std::string BG_BRIGHT_RED = "\033[101m";    // 背景亮红色
    inline std::string BG_BRIGHT_GREEN = "\033[102m";  // 背景亮绿色
    inline std::string BG_BRIGHT_YELLOW = "\033[103m"; // 背景亮黄色
    inline std::string BG_BRIGHT_BLUE = "\033[104m";   // 背景亮蓝色
    inline std::string BG_BRIGHT_MAGENTA = "\033[105m";// 背景亮品红色
    inline std::string BG_BRIGHT_CYAN = "\033[106m";   // 背景亮青色
    inline std::string BG_BRIGHT_WHITE = "\033[107m";  // 背景亮白色
}
