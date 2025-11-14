cmake_minimum_required(VERSION 3.10)

project(kiz
        VERSION 0.1.0 
        LANGUAGES CXX)

# 设置C++标准（根据项目需求选择，如C++11/C++17，这里用C++11兼容更多场景）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制使用指定的C++标准
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用编译器扩展（保证跨平台一致性）

# 收集所有源文件（src及子文件夹下的所有.cpp）
# GLOB_RECURSE：递归查找指定路径下的文件，避免手动列子文件夹文件
file(GLOB_RECURSE SRC_FILES
     "${PROJECT_SOURCE_DIR}/src/*.cpp" # src根目录及子文件夹的.cpp
)

# 检查main.cpp是否存在（避免编译报错）
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/src/main.cpp")
    message(FATAL_ERROR "未找到入口文件：src/main.cpp，请确认文件路径正确！")
endif()

# 添加可执行目标（目标名kiz，关联所有源文件）
add_executable(kiz ${SRC_FILES})

# 设置头文件搜索路径（include、deps，支持子文件夹）
# PRIVATE：仅当前目标使用这些头文件（无需暴露给其他依赖）
target_include_directories(kiz
    PRIVATE
        "${PROJECT_SOURCE_DIR}/include" # include及子文件夹的.hpp
        "${PROJECT_SOURCE_DIR}/deps"    # deps根目录的.hpp（无subfolders）
)

# 按平台设置可执行文件后缀（Windows→.exe，Linux/macOS→.elf）
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    # Windows平台强制添加.exe后缀
    set_target_properties(kiz PROPERTIES
        SUFFIX ".exe"
    )
else()
    # Linux/macOS等平台添加.elf后缀
    set_target_properties(kiz PROPERTIES
        SUFFIX ".elf"
    )
endif()

# 可选：打印编译信息（方便调试路径）
message(STATUS "=== 项目kiz v${PROJECT_VERSION} 编译配置 ===")
message(STATUS "源文件数量：${CMAKE_ARGC}")
message(STATUS "头文件搜索路径：")
message(STATUS "  - ${PROJECT_SOURCE_DIR}/include")
message(STATUS "  - ${PROJECT_SOURCE_DIR}/deps")
message(STATUS "目标文件：${CMAKE_BINARY_DIR}/kiz$<TARGET_FILE_SUFFIX:kiz>")
message(STATUS "======================================")