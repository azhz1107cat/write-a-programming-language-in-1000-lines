#pragma once
#include <iostream>
#include "ui/color.hpp"

// 调试模式开关
//#define IN_DEBUG

#ifdef IN_DEBUG
#define DEBUG_OUTPUT(msg) \
do { \
std::cout << Color::BRIGHT_YELLOW << "[DEBUG] " \
<< __FILE__ << ":" << __LINE__ << " | " \
<< "msg: " << (msg) << Color::RESET << std::endl; \
} while(0)
#else
#define DEBUG_OUTPUT(msg) ((void)0)
#endif