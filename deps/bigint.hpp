/**
 * @file bigint.hpp
 * @brief 无限精度整数（BigInt）核心定义
 *  * 采用逆序存储 digits（如 123 存储为 [3,2,1]），最小化进位/借位时的元素移动开销；
 * 支持 size_t 与合法数字字符串初始化，内置高效比较与 IO 操作。
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once
#include <vector>
#include <string>
#include <ostream>
#include <cstdint>
#include <utility>
#include <algorithm> // 用于 max/min 函数

namespace kiz {

class BigInt {
private:
    std::vector<uint8_t> digits_; // 逆序存储（低位在前），每个元素 0-9
    bool is_negative_;            // false=正，true=负；0恒为非负

    /**
     * @brief 移除前导零（逆序中为末尾零），确保数字表示唯一
     */
    void trim_leading_zeros() {
        while (digits_.size() > 1 && digits_.back() == 0) {
            digits_.pop_back();
        }
        if (digits_.size() == 1 && digits_[0] == 0) {
            is_negative_ = false;
        }
    }

    /**
     * @brief 绝对值比较（辅助减法/乘法）：返回 *this 的绝对值是否小于 other 的绝对值
     */
    bool abs_less(const BigInt& other) const {
        if (digits_.size() != other.digits_.size()) {
            return digits_.size() < other.digits_.size();
        }
        // 从最高位（digits_末尾）逐位比
        for (auto it1 = digits_.rbegin(), it2 = other.digits_.rbegin(); it1 != digits_.rend(); ++it1, ++it2) {
            if (*it1 != *it2) {
                return *it1 < *it2;
            }
        }
        return false; // 绝对值相等
    }

    /**
     * @brief Karatsuba乘法核心（无符号，仅处理正整数）
     * 时间复杂度 O(n^log3) ≈ O(n^1.58)，远快于普通O(n²)逐位乘
     */
    static BigInt karatsuba_mul(const BigInt& a, const BigInt& b) {
        // 基准情况：其中一个数为个位数，直接逐位乘
        if (a.digits_.size() == 1 || b.digits_.size() == 1) {
            BigInt res;
            res.digits_.resize(a.digits_.size() + b.digits_.size(), 0);
            // 逐位相乘，累加至对应位置
            for (size_t i = 0; i < a.digits_.size(); ++i) {
                uint32_t carry = 0; // 用32位存进位，避免溢出
                for (size_t j = 0; j < b.digits_.size(); ++j) {
                    uint32_t sum = res.digits_[i + j] + a.digits_[i] * b.digits_[j] + carry;
                    res.digits_[i + j] = static_cast<uint8_t>(sum % 10);
                    carry = sum / 10;
                }
                if (carry > 0) {
                    res.digits_[i + b.digits_.size()] = static_cast<uint8_t>(carry);
                }
            }
            res.trim_leading_zeros();
            return res;
        }

        // 分治步骤：将 a、b 分为高低位（m 为较小长度的一半，向上取整）
        size_t m = std::max(a.digits_.size(), b.digits_.size()) / 2;
        BigInt a_low, a_high, b_low, b_high;

        // 拆分 a：a_low = a % 10^m，a_high = a / 10^m（逆序存储，前m位是低位）
        a_low.digits_ = std::vector<uint8_t>(a.digits_.begin(), a.digits_.begin() + std::min(m, a.digits_.size()));
        if (a.digits_.size() > m) {
            a_high.digits_ = std::vector<uint8_t>(a.digits_.begin() + m, a.digits_.end());
        } else {
            a_high.digits_ = {0}; // 不足高位补0
        }

        // 拆分 b（同 a）
        b_low.digits_ = std::vector<uint8_t>(b.digits_.begin(), b.digits_.begin() + std::min(m, b.digits_.size()));
        if (b.digits_.size() > m) {
            b_high.digits_ = std::vector<uint8_t>(b.digits_.begin() + m, b.digits_.end());
        } else {
            b_high.digits_ = {0};
        }

        // Karatsuba公式：(a_high*10^m + a_low) * (b_high*10^m + b_low)
        // = z0*10^(2m) + (z1 - z0 - z2)*10^m + z2，其中 z0=a_low*b_low, z1=(a_low+a_high)*(b_low+b_high), z2=a_high*b_high
        BigInt z0 = karatsuba_mul(a_low, b_low);
        BigInt z1 = karatsuba_mul(a_low + a_high, b_low + b_high);
        BigInt z2 = karatsuba_mul(a_high, b_high);

        // 计算结果：z0 + (z1 - z0 - z2)*10^m + z2*10^(2m)
        BigInt res = z0 + (z1 - z0 - z2).shift_left(m) + z2.shift_left(2 * m);
        res.trim_leading_zeros();
        return res;
    }

    /**
     * @brief 左移（乘以 10^k，逆序存储中为末尾补k个0）
     */
    BigInt shift_left(size_t k) const {
        if (k == 0 || (digits_.size() == 1 && digits_[0] == 0)) {
            return *this;
        }
        BigInt res = *this;
        res.digits_.insert(res.digits_.begin(), k, 0); // 逆序左移=开头补0（对应10^k）
        return res;
    }

public:
    // ========================= 构造与析构（同前，补充shift_left友元声明） =========================
    BigInt() : is_negative_(false), digits_(1, 0) {}
    explicit BigInt(size_t val) : is_negative_(false) {
        if (val == 0) { digits_.push_back(0); return; }
        while (val > 0) { digits_.push_back(static_cast<uint8_t>(val % 10)); val /= 10; }
    }
    explicit BigInt(const std::string& s) : is_negative_(false) {
        if (s.empty()) { digits_.push_back(0); return; }
        size_t start_idx = 0;
        if (s[0] == '-') { is_negative_ = true; start_idx = 1; }
        for (auto it = s.rbegin(); it != s.rend() - start_idx; ++it) {
            if (*it < '0' || *it > '9') { digits_ = {0}; is_negative_ = false; return; }
            digits_.push_back(static_cast<uint8_t>(*it - '0'));
        }
        trim_leading_zeros();
    }
    BigInt(BigInt&& other) noexcept : digits_(std::move(other.digits_)), is_negative_(other.is_negative_) {
        other.digits_.clear(); other.is_negative_ = false;
    }
    BigInt& operator=(BigInt&& other) noexcept {
        if (this != &other) { digits_ = std::move(other.digits_); is_negative_ = other.is_negative_; other.digits_.clear(); other.is_negative_ = false; }
        return *this;
    }
    BigInt(const BigInt& other) = default;
    BigInt& operator=(const BigInt& other) = default;
    ~BigInt() = default;


    // ========================= 比较运算符（同前） =========================
    bool operator==(const BigInt& other) const {
        if (is_negative_ != other.is_negative_ || digits_.size() != other.digits_.size()) return false;
        return digits_ == other.digits_;
    }
    bool operator!=(const BigInt& other) const { return !(*this == other); }
    bool operator<(const BigInt& other) const {
        if (is_negative_ != other.is_negative_) return is_negative_;
        if (digits_.size() != other.digits_.size()) return is_negative_ ? (digits_.size() > other.digits_.size()) : (digits_.size() < other.digits_.size());
        for (auto it1 = digits_.rbegin(), it2 = other.digits_.rbegin(); it1 != digits_.rend(); ++it1, ++it2) {
            if (*it1 != *it2) return is_negative_ ? (*it1 > *it2) : (*it1 < *it2);
        }
        return false;
    }
    bool operator>(const BigInt& other) const { return other < *this; }
    bool operator<=(const BigInt& other) const { return *this < other || *this == other; }
    bool operator>=(const BigInt& other) const { return *this > other || *this == other; }


    // ========================= 核心运算：加法 =========================
    /**
     * @brief 加法运算符：分同号/异号处理，复用绝对值比较
     */
    BigInt operator+(const BigInt& other) const {
        BigInt res;
        res.digits_.clear(); // 清空默认的[0]

        // 情况1：同号（都正或都负）→ 绝对值相加，符号不变
        if (is_negative_ == other.is_negative_) {
            res.is_negative_ = is_negative_;
            uint32_t carry = 0; // 进位（用32位避免溢出）
            size_t max_len = std::max(digits_.size(), other.digits_.size());

            for (size_t i = 0; i < max_len || carry > 0; ++i) {
                // 取当前位（不足补0）
                uint32_t a = (i < digits_.size()) ? digits_[i] : 0;
                uint32_t b = (i < other.digits_.size()) ? other.digits_[i] : 0;
                uint32_t sum = a + b + carry;

                res.digits_.push_back(static_cast<uint8_t>(sum % 10));
                carry = sum / 10;
            }
        }
        // 情况2：异号（一正一负）→ 绝对值相减，符号取绝对值大的
        else {
            if (abs_less(other)) { // this绝对值 < other绝对值 → 结果符号=other符号
                res = other - *this;
            } else { // this绝对值 >= other绝对值 → 结果符号=this符号
                res = *this - other;
                res.is_negative_ = is_negative_;
            }
        }

        res.trim_leading_zeros();
        return res;
    }

    /**
     * @brief 加法赋值运算符（复用+，减少拷贝）
     */
    BigInt& operator+=(const BigInt& other) {
        *this = *this + other;
        return *this;
    }


    // ========================= 核心运算：减法 =========================
    /**
     * @brief 减法运算符：确保大减小，避免负数中间结果
     */
    BigInt operator-(const BigInt& other) const {
        // 特殊情况：减自己 → 0
        if (*this == other) {
            return BigInt(0);
        }

        BigInt res;
        res.digits_.clear();
        res.is_negative_ = false;

        // 确定被减数和减数（确保被减数绝对值 >= 减数绝对值）
        const BigInt* minuend = this;  // 被减数
        const BigInt* subtrahend = &other; // 减数
        if (abs_less(other)) {
            minuend = &other;
            subtrahend = this;
            res.is_negative_ = true; // 结果为负
        }

        // 绝对值相减（大减小）
        uint32_t borrow = 0; // 借位
        for (size_t i = 0; i < minuend->digits_.size(); ++i) {
            // 取当前位（减数不足补0）
            uint32_t a = minuend->digits_[i];
            uint32_t b = (i < subtrahend->digits_.size()) ? subtrahend->digits_[i] : 0;
            // 处理借位：当前位不够减，向前借1（变成10+当前位）
            a -= borrow;
            borrow = 0;
            if (a < b) {
                a += 10;
                borrow = 1;
            }
            uint32_t diff = a - b;
            res.digits_.push_back(static_cast<uint8_t>(diff));
        }

        res.trim_leading_zeros();
        return res;
    }

    /**
     * @brief 减法赋值运算符（复用-，减少拷贝）
     */
    BigInt& operator-=(const BigInt& other) {
        *this = *this - other;
        return *this;
    }


    // ========================= 核心运算：乘法 =========================
    /**
     * @brief 乘法运算符：符号单独处理，绝对值用Karatsuba算法计算
     */
    BigInt operator*(const BigInt& other) const {
        // 特殊情况：有一个数为0 → 结果为0
        if ((digits_.size() == 1 && digits_[0] == 0) || (other.digits_.size() == 1 && other.digits_[0] == 0)) {
            return BigInt(0);
        }

        BigInt res;
        // 符号：同号为正，异号为负（异或运算）
        res.is_negative_ = is_negative_ ^ other.is_negative_;
        // 绝对值相乘（调用Karatsuba核心）
        res = karatsuba_mul(*this, other);
        res.trim_leading_zeros();
        return res;
    }

    /**
     * @brief 乘法赋值运算符（复用*，减少拷贝）
     */
    BigInt& operator*=(const BigInt& other) {
        *this = *this * other;
        return *this;
    }


    // ========================= 输出运算符（同前） =========================
    friend std::ostream& operator<<(std::ostream& os, const BigInt& num) {
        if (num.is_negative_) {
            os << '-';
        }
        for (auto it = num.digits_.rbegin(); it != num.digits_.rend(); ++it) {
            os << static_cast<char>('0' + *it);
        }
        return os;
    }
};


// ========================= 使用示例（可直接在代码中测试） =========================
/*
#include <iostream>
int main() {
    // 1. 初始化测试
    BigInt a("123456789012345678901234567890"); // 大正数
    BigInt b(-987654321);                        // 负数（size_t初始化后改符号，或直接字符串"-987654321"）
    BigInt c(123);                               // size_t初始化

    // 2. 加法测试
    BigInt add_res = a + c;
    std::cout << "a + c = " << add_res << std::endl; // 123456789012345678901234567890 + 123 = 123456789012345678901234568013

    // 3. 减法测试
    BigInt sub_res = a - b;
    std::cout << "a - b = " << sub_res << std::endl; // 123456789012345678901234567890 - (-987654321) = 123456789012345678901234568877

    // 4. 乘法测试（大数字高效计算）
    BigInt mul_res = a * c;
    std::cout << "a * c = " << mul_res << std::endl; // 123456789012345678901234567890 * 123 = 15185185048518518504851851850470

    return 0;
}
*/

} // namespace kiz