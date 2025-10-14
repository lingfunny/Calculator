#pragma once

#include <string>
#include <iostream>

using i64 = long long;

struct Fraction {
    i64 numerator;
    i64 denominator;

    Fraction(i64 num = 0, i64 denom = 1);
    void normalize();

    friend Fraction operator+(const Fraction &a, const Fraction &b);
    Fraction& operator+=(const Fraction &other);
    Fraction operator-() const;
    friend Fraction operator-(const Fraction &a, const Fraction &b);
    Fraction& operator-=(const Fraction &other);
    friend Fraction operator*(const Fraction &a, const Fraction &b);
    Fraction& operator*=(const Fraction &other);
    friend Fraction operator/(const Fraction &a, const Fraction &b);
    Fraction& operator/=(const Fraction &other);
    friend Fraction operator^(const Fraction &base, int exponent);
    Fraction& operator^=(int exponent);
};

Fraction expression_evaluate(const std::string &expr);
std::ostream& operator<<(std::ostream &os, const Fraction &value);