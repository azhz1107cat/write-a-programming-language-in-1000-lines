cmake_minimum_required(VERSION 3.10)

project(kiz
        VERSION 0.1.0
        LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(KIZ_VERSION_MAJOR 0)
set(KIZ_VERSION_MINOR 1)
set(KIZ_VERSION_PATCH 0)
set(KIZ_VERSION "${KIZ_VERSION_MAJOR}.${KIZ_VERSION_MINOR}.${KIZ_VERSION_PATCH}")

# 配置模板文件（生成到构建目录的include下）
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/version.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/version.hpp
        @ONLY
)

# 收集所有源文件
file(GLOB_RECURSE SRC_FILES
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# 检查main.cpp是否存在
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/src/main.cpp")
    message(FATAL_ERROR "未找到入口文件：src/main.cpp，请确认文件路径正确！")
endif()

# 添加可执行目标
add_executable(kiz ${SRC_FILES})

# 关键修改：添加构建目录的include到搜索路径
target_include_directories(kiz
        PRIVATE
        "${PROJECT_SOURCE_DIR}/include"       # 源码的include
        "${PROJECT_SOURCE_DIR}/deps"          # deps目录
        "${CMAKE_CURRENT_BINARY_DIR}/include" # 构建目录的include（生成的version.hpp在这里）
)

# 按平台设置可执行文件后缀
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set_target_properties(kiz PROPERTIES SUFFIX ".exe")
else()
    set_target_properties(kiz PROPERTIES SUFFIX ".elf")
endif()

# 修正打印信息
message(STATUS "=== 项目kiz v${PROJECT_VERSION} 编译配置 ===")
message(STATUS "源文件数量：${CMAKE_ARGC}")
message(STATUS "头文件搜索路径：")
message(STATUS "  - ${PROJECT_SOURCE_DIR}/include")
message(STATUS "  - ${PROJECT_SOURCE_DIR}/deps")
message(STATUS "  - ${CMAKE_CURRENT_BINARY_DIR}/include") # 新增：打印构建目录的include路径
message(STATUS "目标文件：${CMAKE_BINARY_DIR}/kiz$<TARGET_FILE_SUFFIX:kiz>")
message(STATUS "======================================")