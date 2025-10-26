/**
 * @file hashmap.hpp
 * @brief 辅助容器（HashMap）核心定义与实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <utility>
#include <variant>

namespace deps {

// 字符串哈希函数（FNV-1a算法）
inline size_t hash_string(const std::string& key) {
    constexpr size_t FNV_OFFSET = 14695981039346656037ULL;
    constexpr size_t FNV_PRIME = 1099511628211ULL;
    size_t hash = FNV_OFFSET;
    for (const char c : key) {
        hash ^= static_cast<size_t>(c);  // 异或当前字符
        hash *= FNV_PRIME;               // 乘以质数
    }
    return hash;
}

// 模板类：键为std::string，值为任意类型T的HashMap
template <typename KT, typename VT>
class HashMap {
private:
    // 嵌套桶节点结构体（存储键值对、哈希值、链表指针）
    struct StringBucket {
        std::string key;
        VT value;
        size_t hash;                        // 缓存哈希值，避免重复计算
        std::shared_ptr<StringBucket> next; // 解决哈希冲突的链表指针

        // 构造函数（移动语义优化）
        StringBucket(std::string k, VT val)
            : key(std::move(k)),
              value(std::move(val)),
              hash(hash_string(key)),
              next(nullptr) {}
    };

    using Node = StringBucket;  // 简化节点类型名
    std::vector<std::shared_ptr<Node>> buckets_;  // 桶数组（每个元素是链表头指针）
    size_t elem_count_ = 0;                       // 元素总数
    const float load_factor_ = 0.7f;              // 负载因子（触发扩容的阈值）

    // 计算键对应的桶索引（利用位运算，要求桶大小为2的幂）
    [[nodiscard]] size_t getBucketIndex(const size_t hash) const {
        return hash & (this->buckets_.size() - 1);
    }

    // 扩容：桶数组大小翻倍，重新哈希所有元素
    void resize() {
        const size_t old_size = buckets_.size();
        const size_t new_size = old_size * 2;
        std::vector<std::shared_ptr<Node>> new_buckets(new_size, nullptr);

        // 遍历旧桶，重新分配元素到新桶
        for (size_t i = 0; i < old_size; ++i) {
            std::shared_ptr<Node> current = buckets_[i];
            while (current != nullptr) {
                const std::shared_ptr<Node> next = current->next;
                const size_t new_idx = getBucketIndex(current->hash);

                // 头插法插入新桶
                current->next = new_buckets[new_idx];
                new_buckets[new_idx] = current;

                current = next;
            }
        }

        buckets_.swap(new_buckets);  // 替换为新桶数组
    }

public:
    // 默认构造函数（初始桶大小为16，2的幂）
    explicit HashMap() {
        constexpr size_t init_size = 16;
        buckets_.resize(init_size, nullptr);
    }

    // 用键值对vector初始化
    explicit HashMap(const std::vector<std::pair<std::string, VT>>& vec) {
        // 计算初始桶大小（确保负载因子不超过阈值）
        size_t init_size = 16;
        while (init_size < (vec.size() / load_factor_)) {
            init_size *= 2;
        }
        buckets_.resize(init_size, nullptr);

        // 插入所有初始键值对
        for (const auto& [key, val] : vec) {
            insert(key, val);
        }
    }

    // 析构函数（默认即可，shared_ptr自动释放内存）
    ~HashMap() = default;

    // 插入/更新键值对（存在则更新，不存在则插入）
    VT insert(const std::string& key, VT val) {
        // 若桶为空，初始化桶大小为16
        if (buckets_.empty()) {
            buckets_.resize(16, nullptr);
        }

        // 检查负载因子，超过阈值则扩容
        if (static_cast<float>(elem_count_) / buckets_.size() >= load_factor_) {
            this->resize();
        }

        const size_t hash = hash_string(key);
        const size_t bucket_idx = getBucketIndex(hash);

        // 检查键是否已存在，存在则更新值
        std::shared_ptr<Node> current = buckets_[bucket_idx];
        while (current != nullptr) {
            if (current->hash == hash && current->key == key) {
                current->value = std::move(val);
                return VT();  // 返回默认构造的T（对应原代码LAMINA_NULL）
            }
            current = current->next;
        }

        // 键不存在，创建新节点（头插法）
        auto new_node = std::make_shared<Node>(key, std::move(val));
        new_node->next = buckets_[bucket_idx];
        buckets_[bucket_idx] = new_node;
        elem_count_++;

        return VT();  // 返回默认构造的T
    }

    // 递归查找键（支持查找父结构体__parent__）
    [[nodiscard]] std::shared_ptr<Node> find(const std::string& key) const {
        // 先在当前HashMap查找
        std::shared_ptr<Node> target_node = find_in_current(key);
        if (target_node != nullptr) {
            return target_node;
        }

        // 未找到，查找__parent__节点（原代码逻辑保留）
        const std::shared_ptr<Node> parent_node = find_in_current("__parent__");
        if (parent_node == nullptr) {
            return nullptr;
        }

        // 检查__parent__的值是否为HashMap指针（需T支持is_lstruct()和data成员）
        if (!parent_node->value.is_lstruct()) {
            return nullptr;
        }
        // 从T的data中提取HashMap指针（原代码依赖std::variant，需T的data为variant类型）
        const auto parent_map_ptr = std::get_if<std::shared_ptr<HashMap<VT>>>(&parent_node->value.data);
        if (parent_map_ptr == nullptr || *parent_map_ptr == nullptr) {
            return nullptr;
        }

        // 递归查找父HashMap
        return (*parent_map_ptr)->find(key);
    }

    // 仅在当前HashMap查找键（不递归父结构体）
    [[nodiscard]] std::shared_ptr<Node> find_in_current(const std::string& key) const {
        if (buckets_.empty()) {
            return nullptr;
        }

        const size_t hash = hash_string(key);
        const size_t bucket_idx = getBucketIndex(hash);
        if (bucket_idx >= buckets_.size()) {
            return nullptr;
        }

        // 遍历桶对应的链表查找
        std::shared_ptr<Node> current = buckets_[bucket_idx];
        while (current != nullptr) {
            if (current->hash == hash && current->key == key) {
                return current;
            }
            current = current->next;
        }

        return nullptr;
    }

    // 转换为字符串（需T支持to_string()成员函数）
    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        ss << "{ ";

        // 统计总元素数（用于处理逗号）
        size_t total_count = 0;
        for (const auto& bucket_head : buckets_) {
            auto current = bucket_head;
            while (current != nullptr) {
                total_count++;
                current = current->next;
            }
        }

        // 拼接所有键值对
        size_t current_idx = 0;
        for (const auto& bucket_head : buckets_) {
            auto current = bucket_head;
            while (current != nullptr) {
                ss << current->key << ": " << current->value.to_string();
                // 非最后一个元素加逗号
                if (current_idx < total_count - 1) {
                    ss << ", ";
                }
                current = current->next;
                current_idx++;
            }
        }

        ss << " }";
        return ss.str();
    }

    // 转换为键值对vector
    [[nodiscard]] std::vector<std::pair<std::string, VT>> to_vector() const {
        std::vector<std::pair<std::string, VT>> vec;
        for (const auto& bucket_head : buckets_) {
            auto current = bucket_head;
            while (current != nullptr) {
                vec.emplace_back(current->key, current->value);
                current = current->next;
            }
        }
        return vec;
    }

    // 深拷贝构造函数
    HashMap(const HashMap& other)
        : load_factor_(other.load_factor_) {
        // 获取源HashMap的所有键值对
        auto all_kv = other.to_vector();
        // 计算初始桶大小
        size_t init_size = 16;
        while (init_size < (all_kv.size() / load_factor_)) {
            init_size *= 2;
        }
        buckets_.resize(init_size, nullptr);

        // 插入所有键值对（深拷贝）
        for (auto& [key, val] : all_kv) {
            this->insert(key, val);
        }
        this->elem_count_ = other.elem_count_;
    }
};

} // namespace deps