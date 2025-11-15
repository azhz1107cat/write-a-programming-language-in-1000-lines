#pragma once
#include <iostream>

// 调试模式开关
// #define IN_DEBUG

#ifdef IN_DEBUG
#define DEBUG_OUTPUT(msg) \
do { \
std::cout << "[DEBUG] " \
<< __FILE__ << ":" << __LINE__ << " | " \
<< "msg: " << (msg) << std::endl; \
} while(0)
#else
#define DEBUG_OUTPUT(msg) ((void)0)
#endif