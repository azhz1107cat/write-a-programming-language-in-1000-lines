#pragma once
#include <iostream> 

#define IN_DEBUG

#ifdef IN_DEBUG
    #define DEBUG(msg) \
        do { \
            std::cout << "[DEBUG] " << __FILE__ << ":" << __LINE__ << " | " << msg << std::endl; \
        } while(0)
#else
    #define DEBUG(...)
#endif