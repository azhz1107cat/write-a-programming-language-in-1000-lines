-- 项目基础配置：名称、版本、语言（仅C++）
set_project("kiz", "0.1.0", "cxx")

-- 全局C++标准配置（强制C++20，与原CMake保持一致）
set_languages("cxx20")
-- 禁用编译器扩展（匹配CMake的 CMAKE_CXX_EXTENSIONS OFF，保证跨平台标准一致性）
set_policy("build.cxxextensions", false)

-- 添加可执行目标（类型为binary，目标名为kiz）
add_target("kiz")
    -- 目标类型：可执行文件
    set_kind("binary")

    -- 收集源文件（递归匹配src及子文件夹下所有.cpp，**表示递归子目录）
    local src_files = os.glob("src/**.cpp")
    -- 检查入口文件main.cpp是否存在，不存在则报错终止
    assert(os.exists("src/main.cpp"), "未找到入口文件：src/main.cpp，请确认文件路径正确！")
    add_files(src_files)

    -- 配置头文件搜索路径（include/deps，仅当前目标可见，匹配CMake的PRIVATE）
    add_includedirs(
        "include",  -- 自动递归搜索include下子文件夹（支持#include "core/config.hpp"这类引用）
        "deps"      -- deps无subfolders，直接搜索根目录
    )
    -- 设置头文件路径作用域为PRIVATE（仅当前目标使用，不暴露给依赖）
    set_includedirs_policy("private")

    -- 按平台设置可执行文件后缀（Windows→.exe，其他→.elf，与原CMake逻辑一致）
    if is_plat("windows") then
        set_suffix(".exe")
    else
        set_suffix(".elf")
    end

    -- 编译信息打印（调试用，匹配原CMake的STATUS日志）
    on_config(function(target)
        print("=== 项目kiz v" .. get_project_version() .. " 编译配置 ===")
        -- 打印源文件数量（#src_files获取数组长度）
        print("源文件数量：" .. tostring(#src_files))
        -- 打印头文件搜索路径
        print("头文件搜索路径：")
        local inc_dirs = target:get_includedirs()
        for _, dir in ipairs(inc_dirs) do
            print("  - " .. dir)
        end
        -- 打印最终目标文件路径（target:targetfile()获取完整路径）
        print("目标文件：" .. target:targetfile())
        print("======================================")
    end)