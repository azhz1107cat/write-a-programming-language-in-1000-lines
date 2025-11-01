/**
 * @file rational.hpp
 * @brief 有理数（Rational）核心定义与实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

#include <stdexcept>
#include "bigint.hpp"

namespace deps {

class BigInt;

class Rational {
    BigInt numerator_;   // 分子
    BigInt denominator_; // 分母（始终为正，通过 reduce 保证）
public:
    // 构造函数（自动约分，保证分母为正）
    // 默认构造：0/1
    Rational() : numerator_(0), denominator_(1) {}
    // 整数构造（分母默认为1）
    explicit Rational(const BigInt& numerator) 
        : numerator_(numerator), denominator_(1) {}
    // 分子+分母构造（核心构造，自动处理符号和约分）
    Rational(const BigInt& numerator, const BigInt& denominator) 
        : numerator_(numerator), denominator_(denominator) {
        reduce(); // 构造时立即约分并规范符号
    }

    // 访问接口（只读，防止外部修改分子分母）
    const BigInt& getNumerator() const { return numerator_; }
    const BigInt& getDenominator() const { return denominator_; }

    // 核心运算符重载（基于 BigInt 已重载的 +-*%）
    // 赋值运算符
    Rational& operator=(const Rational& rhs) {
        if (this != &rhs) {
            numerator_ = rhs.numerator_;
            denominator_ = rhs.denominator_;
        }
        return *this;
    }

    // 加法：a/b + c/d = (a*d + c*b)/(b*d)
    Rational operator+(const Rational& rhs) const {
        return Rational(
            numerator_ * rhs.denominator_ + rhs.numerator_ * denominator_,
            denominator_ * rhs.denominator_
        );
    }

    // 减法：a/b - c/d = (a*d - c*b)/(b*d)
    Rational operator-(const Rational& rhs) const {
        return Rational(
            numerator_ * rhs.denominator_ - rhs.numerator_ * denominator_,
            denominator_ * rhs.denominator_
        );
    }

    // 乘法：a/b * c/d = (a*c)/(b*d)
    Rational operator*(const Rational& rhs) const {
        return Rational(
            numerator_ * rhs.numerator_,
            denominator_ * rhs.denominator_
        );
    }

    // 除法：a/b ÷ c/d = (a*d)/(b*c)（需检查除数不为0）
    Rational operator/(const Rational& rhs) const {
        if (rhs.numerator_ == BigInt(0)) {
            throw std::invalid_argument("Rational division by zero");
        }
        return Rational(
            numerator_ * rhs.denominator_,
            denominator_ * rhs.numerator_
        );
    }

    // 比较运算符（基于交叉相乘，避免浮点精度问题）
    bool operator==(const Rational& rhs) const {
        // 分母已规范为正，直接交叉相乘比较
        return numerator_ * rhs.denominator_ == rhs.numerator_ * denominator_;
    }

    bool operator!=(const Rational& rhs) const {
        return !(*this == rhs);
    }

private:

    // 辅助函数：计算两个 BigInt 的最大公约数（欧几里得算法）
    static BigInt gcd(const BigInt& a, const BigInt& b) {
        BigInt x = a.abs(); // 取绝对值，确保 gcd 为正
        BigInt y = b.abs();
        while (y != BigInt(0)) {
            BigInt temp = y;
            y = x % y; // 依赖 BigInt 已重载的 % 运算符
            x = temp;
        }
        return x;
    }

    // 辅助函数：约分并规范符号（分母为正，分子带符号）
    void reduce() {
        // 检查分母是否为0
        if (denominator_ == BigInt(0)) {
            throw std::invalid_argument("Rational denominator cannot be zero");
        }

        // 规范分母符号（分母为负则将负号转移到分子）
        if (denominator_ < BigInt(0)) {
            numerator_ = -numerator_;
            denominator_ = -denominator_;
        }

        // 约分（分子分母同除以最大公约数）
        BigInt g = gcd(numerator_, denominator_);
        if (g != BigInt(0)) { // 避免分子为0时 gcd 异常
            numerator_ /= g;
            denominator_ /= g;
        }

        // 第四步：特殊处理分子为0的情况（分母强制为1）
        if (numerator_ == BigInt(0)) {
            denominator_ = BigInt(1);
        }
    }
};

// 实现 BigInt 除法返回 Rational（核心需求）
// 重载 BigInt 的 / 运算符，直接返回 Rational 对象
inline Rational operator/(const BigInt& lhs, const BigInt& rhs) {
    return Rational(lhs, rhs); // 复用 Rational 双参数构造（自动约分）
}

} // namespace deps