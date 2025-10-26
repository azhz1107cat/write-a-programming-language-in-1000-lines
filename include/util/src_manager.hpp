#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <filesystem>

namespace fs = std::filesystem;

namespace util {

// path: content
inline std::unordered_map<std::string, std::string> opened_files; 

inline std::string get_slice(const std::string& src_path, const int& src_line_start, const int& src_line_end);

// 获取opened_files中的文件(线程安全)
std::string get_file_at_path(const std::string& path);

// 打开lm文件并将其添加到opened_files, 返回文件内容
std::string open_lm_file(const std::string& path);

} // namespace util