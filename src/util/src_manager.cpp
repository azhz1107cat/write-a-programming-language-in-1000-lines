/**
 * @file src_manager.cpp
 * @brief 源文件管理器（Source File Manager）核心
 * 管理Kiz代码源文件
 * 并给予错误报告器源文件错误代码的切片
 * @author azhz1107cat
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <iterator>
#include <algorithm>

namespace util {

// 全局文件缓存：键为文件路径，值为文件内容
inline std::unordered_map<std::string, std::string> opened_files;
// 全局互斥锁：保证多线程下对opened_files的安全访问
inline std::mutex opened_files_mutex;

/**
 * @brief 从指定文件中提取指定行范围的内容（行号从1开始）
 * @param src_path 文件路径
 * @param src_line_start 起始行号（闭区间）
 * @param src_line_end 结束行号（闭区间）
 * @return 提取的行内容（每行保留原始换行符，无效范围返回空字符串）
 */
std::string get_slice(const std::string& src_path, const int& src_line_start, const int& src_line_end) {
    // 先获取完整文件内容（依赖缓存机制）
    std::string file_content = get_file_by_path(src_path);
    if (file_content.empty()) {
        return "";
    }

    // 按行分割文件内容（兼容 Windows \r\n 和 Linux \n 换行符）
    std::vector<std::string> lines;
    std::stringstream content_stream(file_content);
    std::string line;
    while (std::getline(content_stream, line)) {
        // 移除 Windows 换行符残留的 \r（std::getline 会自动去掉 \n，但不会处理 \r）
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.emplace_back(std::move(line));
    }

    // 校验行号范围有效性（行号从1开始，且起始行 ≤ 结束行）
    const size_t total_lines = lines.size();
    if (src_line_start < 1 || src_line_end < 1 || 
        src_line_start > src_line_end || 
        static_cast<size_t>(src_line_end) > total_lines) {
        std::cerr << "[Warning] Invalid line range: start=" << src_line_start 
                  << ", end=" << src_line_end << " (total lines: " << total_lines << ")\n";
        return "";
    }

    // 提取目标行范围（转换为0-based索引）
    std::stringstream slice_stream;
    const size_t start_idx = static_cast<size_t>(src_line_start) - 1;
    const size_t end_idx = static_cast<size_t>(src_line_end) - 1;
    for (size_t i = start_idx; i <= end_idx; ++i) {
        slice_stream << lines[i];
        // 除最后一行外，保留原始换行符（还原文件格式）
        if (i != end_idx) {
            slice_stream << '\n';
        }
    }

    return slice_stream.str();
}

/**
 * @brief 线程安全获取文件内容（优先从缓存读取，未命中则新打开）
 * @param path 文件路径
 * @return 文件完整内容（打开失败会抛出异常）
 */
std::string get_file_by_path(const std::string& path) {
    // 加锁保证多线程下缓存操作的原子性
    std::lock_guard<std::mutex> lock(opened_files_mutex);

    // 检查缓存是否已存在该文件
    auto iter = opened_files.find(path);
    if (iter != opened_files.end()) {
        return iter->second;
    }

    // 缓存未命中，新打开文件并加入缓存
    std::string file_content = open_new_kiz_file(path);
    opened_files.emplace(path, file_content);

    return file_content;
}

/**
 * @brief 打开Kiz文件并读取内容（不直接对外暴露，由get_file_by_path调用）
 * @param path 文件路径
 * @return 文件完整内容（打开失败会抛出std::runtime_error）
 */
std::string open_new_file(const std::string& path) {
    // 以文本模式打开文件（自动处理换行符转换，避免二进制模式的乱码问题）
    std::ifstream kiz_file(path, std::ios::in | std::ios::binary);
    if (!kiz_file.is_open()) {
        throw std::runtime_error("Failed to open kiz file: " + path + 
                                 " (reason: " + std::strerror(errno) + ")");
    }

    // 读取文件全部内容（高效读取方式，避免逐行读取的性能损耗）
    std::string file_content;
    // 调整流缓冲区大小以优化大文件读取
    kiz_file.seekg(0, std::ios::end);
    file_content.reserve(static_cast<size_t>(kiz_file.tellg()));
    kiz_file.seekg(0, std::ios::beg);

    // 用迭代器读取全部字符（兼容空文件场景）
    file_content.assign(std::istreambuf_iterator<char>(kiz_file),
                        std::istreambuf_iterator<char>());

    // 关闭文件（ifstream析构时会自动关闭，显式关闭更严谨）
    kiz_file.close();

    return file_content;
}

} // namespace util