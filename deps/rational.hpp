/**
 * @file rational.hpp
 * @brief 有理数（Rational）核心定义与实现
 * 
 * @author azhz1107cat
 * @date 2025-10-25
 */

#pragma once

#include <stdexcept>
#include <utility>

namespace deps {

class BigInt;

class Rational {
public:
    BigInt numerator;   // 分子
    BigInt denominator; // 分母（始终为正，通过 reduce 保证）

    // 构造函数（自动约分，保证分母为正）
    // 默认构造：0/1
    Rational() : numerator(0), denominator(1) {}
    // 整数构造（分母默认为1）
    explicit Rational(BigInt  numerator)
        : numerator(std::move(numerator)), denominator(1) {}
    // 分子+分母构造（核心构造，自动处理符号和约分）
    Rational(BigInt  numerator, BigInt  denominator)
        : numerator(std::move(numerator)), denominator(std::move(denominator)) {
        reduce(); // 构造时立即约分并规范符号
    }

    // 访问接口（只读，防止外部修改分子分母）
    [[nodiscard]] const BigInt& getNumerator() const { return numerator; }
    [[nodiscard]] const BigInt& getDenominator() const { return denominator; }

    // 核心运算符重载（基于 BigInt 已重载的 +-*%）
    // 赋值运算符
    Rational& operator=(const Rational& rhs) {
        if (this != &rhs) {
            numerator = rhs.numerator;
            denominator = rhs.denominator;
        }
        return *this;
    }

    // 加法：a/b + c/d = (a*d + c*b)/(b*d)
    Rational operator+(const Rational& rhs) const {
        return Rational(
            numerator * rhs.denominator + rhs.numerator * denominator,
            denominator * rhs.denominator
        );
    }

    // 减法：a/b - c/d = (a*d - c*b)/(b*d)
    Rational operator-(const Rational& rhs) const {
        return Rational(
            numerator * rhs.denominator - rhs.numerator * denominator,
            denominator * rhs.denominator
        );
    }

    // 乘法：a/b * c/d = (a*c)/(b*d)
    Rational operator*(const Rational& rhs) const {
        return Rational(
            numerator * rhs.numerator,
            denominator * rhs.denominator
        );
    }

    // 除法：a/b ÷ c/d = (a*d)/(b*c)（需检查除数不为0）
    Rational operator/(const Rational& rhs) const {
        if (rhs.numerator == BigInt(0)) {
            throw std::invalid_argument("Rational division by zero");
        }
        return Rational(
            numerator * rhs.denominator,
            denominator * rhs.numerator
        );
    }

    // 比较运算符（基于交叉相乘，避免浮点精度问题）
    bool operator==(const Rational& rhs) const {
        // 分母已规范为正，直接交叉相乘比较
        return numerator * rhs.denominator == rhs.numerator * denominator;
    }

    bool operator!=(const Rational& rhs) const {
        return !(*this == rhs);
    }

    // 小于运算符：a/b < c/d → a*d < c*b（分母b、d均为正，不等号方向不变）
    bool operator<(const Rational& rhs) const {
        // 交叉相乘比较
        return numerator * rhs.denominator < rhs.numerator * denominator;
    }

    // 大于运算符：a/b > c/d → a*d > c*b
    bool operator>(const Rational& rhs) const {
        return numerator * rhs.denominator > rhs.numerator * denominator;
    }

    bool operator<=(const Rational& rhs) const {
        return !(*this > rhs); // 等价于 (*this < rhs) || (*this == rhs)
    }

    bool operator>=(const Rational& rhs) const {
        return !(*this < rhs); // 等价于 (*this > rhs) || (*this == rhs)
    }

private:

    // 辅助函数：计算两个 BigInt 的最大公约数（欧几里得算法）
    BigInt gcd(const BigInt& a, const BigInt& b) {
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
        if (denominator == BigInt(0)) {
            throw std::invalid_argument("Rational denominator cannot be zero");
        }

        // 规范分母符号（分母为负则将负号转移到分子）
        if (denominator < BigInt(0)) {
            numerator = numerator * BigInt(-1);
            denominator = denominator.abs();
        }

        // 约分（分子分母同除以最大公约数）
        BigInt g = gcd(numerator, denominator);
        // 去掉 g != 0 判断（gcd 结果不可能为0：0和x的gcd是x的绝对值，x非0）
        numerator = numerator / g;
        denominator = denominator / g; // 修复：原来是 numerator / g

        // 特殊处理分子为0的情况（分母强制为1）
        if (numerator == BigInt(0)) {
            denominator = BigInt(1);
        }
    }
};

} // namespace deps

// 实现 BigInt 除法返回 Rational（核心需求）
// 重载 BigInt 的 / 运算符，直接返回 Rational 对象
inline deps::Rational operator/(const deps::BigInt& lhs, const deps::BigInt& rhs) {
    return {lhs, rhs}; // 复用 Rational 双参数构造（自动约分）
}