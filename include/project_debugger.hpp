#pragma once
#include <iostream> 

#define IN_DEBUG

#define DEBUG_OUTPUT(msg) \
    do { \
        std::cout << "[DEBUG] " << __FILE__ << ":" << __LINE__ << " | msg: " << msg << std::endl; \
    } while(0)