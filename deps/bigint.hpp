/**
 * @file bigint.hpp
 * @brief 无限精度整数（BigInt）核心定义
 *  * 采用逆序存储 digits（如 123 存储为 [3,2,1]），最小化进位/借位时的元素移动开销；
 * 支持 size_t 与合法数字字符串初始化，内置高效比较与 IO 操作。
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once
#include <algorithm> // 鐢ㄤ簬 max/min 鍑芥暟
#include <cassert>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace deps {

class BigInt {
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
    [[nodiscard]] bool abs_less(const BigInt& other) const {
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
     * @brief 核心辅助：计算 (dividend / divisor) 的商和余数（无符号，仅处理正整数）
     * @param dividend 被除数（非负）
     * @param divisor 除数（正，非零）
     * @return  pair<商, 余数>（均非负）
     */
    static std::pair<BigInt, BigInt> div_mod_unsigned(const BigInt& dividend, const BigInt& divisor) {
        BigInt quotient, remainder;
        quotient.digits_.clear();
        remainder.digits_.clear();
        remainder.is_negative_ = false;
        quotient.is_negative_ = false;

        // 从被除数最高位（digits_末尾）开始逐位构建余数
        for (auto it = dividend.digits_.rbegin(); it != dividend.digits_.rend(); ++it) {
            // 余数 = 余数 * 10 + 当前位（模拟手工除法）
            remainder = remainder * BigInt(10) + BigInt(static_cast<size_t>(*it));

            // 计算当前位商：找到最大的k使 k*divisor <= remainder
            uint8_t q_digit = 0;
            BigInt temp = divisor;
            while (temp <= remainder) {
                q_digit++;
                temp += divisor;
            }

            // 商的当前位存入（注意商是正序构建，后续需逆序）
            quotient.digits_.push_back(q_digit);
            // 更新余数：remainder = remainder - (q_digit * divisor)
            if (q_digit > 0) {
                remainder -= (divisor * BigInt(static_cast<size_t>(q_digit)));
            }
        }

        // 商是正序存储（高位在前），需逆转为统一的低位在前格式
        std::reverse(quotient.digits_.begin(), quotient.digits_.end());
        quotient.trim_leading_zeros();
        remainder.trim_leading_zeros();

        return {quotient, remainder};
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

    /**
    * @brief 辅助函数：无符号快速幂（底数和指数均为非负整数）
    * 二分幂核心逻辑：a^b = (a^(b/2))^2 （b为偶数） / (a^(b/2))^2 * a （b为奇数）
    */
    static BigInt fast_pow_unsigned(const BigInt& base, const BigInt& exp) {
        BigInt result(1); // 初始结果为 1（乘法单位元）
        BigInt current_base = base;
        BigInt current_exp = exp;
        const BigInt two(2);

        while (current_exp > BigInt(0)) {
            // 若当前指数为奇数，结果 *= 当前底数
            if (current_exp % two == BigInt(1)) {
                result = result * current_base; // 复用 Karatsuba 乘法，高效
            }
            // 底数平方，指数减半（整除）
            current_base = current_base * current_base;
            current_exp = current_exp / two; // 整数除法，向下取整
        }

        return result;
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


    [[nodiscard]] BigInt abs() const {
        BigInt res = *this;
        res.is_negative_ = false;
        return res;
    }

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

    // ========================= 核心运算：取模 =========================
    /**
     * @brief 取模运算符：a mod b
     * 规则：1. 除数b不能为0（断言报错）；2. 结果符号与被除数a一致；3. 0 ≤ |结果| < |b|
     */
    BigInt operator%(const BigInt& other) const {
        // 断言：除数不能为0
        assert(!(other.digits_.size() == 1 && other.digits_[0] == 0) && "BigInt mod: divisor cannot be zero");

        // 特殊情况：被除数为0→结果为0
        if (digits_.size() == 1 && digits_[0] == 0) {
            return BigInt(0);
        }

        // 计算被除数、除数的绝对值
        BigInt a_abs = this->abs();
        BigInt b_abs = other.abs();

        // 无符号取模（得到非负余数）
        BigInt remainder = div_mod_unsigned(a_abs, b_abs).second;

        // 调整余数符号（与被除数一致）
        if (this->is_negative_ && remainder != BigInt(0)) {
            remainder = remainder - b_abs; // 非零负余数：remainder = - (b_abs - remainder)
        }

        remainder.trim_leading_zeros();
        return remainder;
    }


    /**
     * @brief 取模赋值运算符：a %= b → a = a mod b
     */
    BigInt& operator%=(const BigInt& other) {
        *this = *this % other;
        return *this;
    }

    /**
     * @brief 除法运算符：返回整数商（向下取整），符号规则同乘法
     */
    BigInt operator/(const BigInt& other) const {
        // 断言：除数不能为0
        assert(!(other.digits_.size() == 1 && other.digits_[0] == 0) && "BigInt division: divisor cannot be zero");

        // 特殊情况：被除数为0→结果为0
        if (digits_.size() == 1 && digits_[0] == 0) {
            return BigInt(0);
        }

        // 符号：同号为正，异号为负（异或运算）
        const bool res_neg = is_negative_ ^ other.is_negative_;

        // 无符号除法取商
        BigInt quotient = div_mod_unsigned(this->abs(), other.abs()).first;
        quotient.is_negative_ = res_neg;
        quotient.trim_leading_zeros();

        return quotient;
    }

    /**
    * @brief 除法赋值运算符
    */
    BigInt& operator/=(const BigInt& other) {
        *this = *this / other;
        return *this;
    }

    /**
     * @brief 幂运算：计算 base^exp（this 为底数，other 为指数）
     * 规则：
     * 1. 指数必须为非负整数（负指数会导致分数，BigInt 不支持，断言报错）；
     * 2. 任何数的 0 次幂 = 1（包括 0^0，编程中默认处理为 1）；
     * 3. 0 的正次幂 = 0；
     * 4. 负数的偶次幂为正，奇次幂为负。
     */
    BigInt pow(const BigInt& other) const {
        // 指数必须为非负整数
        assert(!other.is_negative_ && "BigInt pow: 指数不支持负数（BigInt 仅存整数）");

        const BigInt& exp = other;
        const BigInt zero(0);
        const BigInt one(1);

        // 边界情况：指数为 0 → 结果为 1
        if (exp == zero) {
            return one;
        }

        // 边界情况：底数为 0 → 结果为 0（指数为正）
        if (*this == zero) {
            return zero;
        }

        // 提取底数的绝对值（后续统一计算正数幂，最后处理符号）
        BigInt base_abs = this->abs();
        // 快速幂计算（处理正整数幂）
        BigInt result_abs = fast_pow_unsigned(base_abs, exp);
        // 确定结果符号（仅当底数为负且指数为奇数时，结果为负）
        bool is_result_neg = this->is_negative_ && (exp % BigInt(2) == one);
        result_abs.is_negative_ = is_result_neg;

        result_abs.trim_leading_zeros();
        return result_abs;
    }

    // ========================= to_string =========================
    [[nodiscard]] std::string to_string() const {
        std::string result;

        // 边界处理：若 digits_ 为空（未初始化/零值），直接返回 "0"
        if (digits_.empty()) {
            return "0";
        }

        // 处理负数符号
        if (is_negative_) {
            result += '-';
        }

        // 逆序遍历
        for (auto it = digits_.rbegin(); it != digits_.rend(); ++it) {
            // *it 是 0-9 的数字，+'0' 转换为对应的 ASCII 字符（如 3 → '3'）
            result += static_cast<char>('0' + *it);
        }

        return result;
    }
};

} // namespace deps