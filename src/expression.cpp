#include "expression.hpp"
#include "stack.hpp"

#include <cctype>
#include <cstddef>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {

std::string remove_spaces(std::string expr) {
    std::string result;
    for (const char &ch : expr) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            result.push_back(ch);
        }
    }
    return result;
}

int precedence(char op) {
	switch (op) {
	case '+':
	case '-':
		return 1;
	case '*':
	case '/':
		return 2;
	case '^':
		return 3;
	default:
		throw std::runtime_error("unknown operator");
	}
}

bool is_unary(char op, const std::string &input, std::size_t index) {
    return (op == '+' || op == '-') && (index == 0 || input[index - 1] == '(' || input[index - 1] == '+' || input[index - 1] == '-' || input[index - 1] == '*' || input[index - 1] == '/' || input[index - 1] == '^');
}

bool is_right_associative(char op) {
	return op == '^';
}

i64 parse_integer(const std::string &text, std::size_t &index) {
    // parse a non-negative integer starting at text[index]
    if (index >= text.size()) {
        throw std::runtime_error("expected digit");
    }
    if (!std::isdigit(static_cast<unsigned char>(text[index]))) {
        throw std::runtime_error("expected digit");
    }
    i64 value = 0;
    while (index < text.size() && std::isdigit(static_cast<unsigned char>(text[index]))) {
        value = value * 10 + (text[index] - '0');
        ++index;
    }
    return value;
}

void apply_operator(Stack<Fraction> &values, char op) {
    // apply operator op to the top two values on the stack
	if (values.size() < 2) {
		throw std::runtime_error("insufficient operands");
	}
	Fraction rhs = values.pop();
	Fraction lhs = values.pop();
	switch (op) {
	case '+':
		values.push(lhs + rhs);
		break;
	case '-':
		values.push(lhs - rhs);
		break;
	case '*':
		values.push(lhs * rhs);
		break;
	case '/':
		values.push(lhs / rhs);
		break;
	case '^':
		if (rhs.denominator != 1) {
			throw std::runtime_error("exponent must be integer");
		}
		values.push(lhs ^ static_cast<int>(rhs.numerator));
		break;
	default:
		throw std::runtime_error("unknown operator");
	}
}

void process_operator(Stack<Fraction> &values, Stack<char> &operators, char op) {
    // process operator op
	while (!operators.empty()) {
		char top = operators.top();
		if (top == '(') {
			break;
		}
		int top_prec = precedence(top);
		int op_prec = precedence(op);
		if (top_prec > op_prec || (top_prec == op_prec && !is_right_associative(op))) {
            // if top operator has higher or equal precedence, apply it first
			operators.pop();
			apply_operator(values, top);
		} else {
			break;
		}
	}
	operators.push(op);
}

void collapse(Stack<Fraction> &values, Stack<char> &operators) {
    // collapse until the matching '('
	while (!operators.empty() && operators.top() != '(') {
		char op = operators.pop();
		apply_operator(values, op);
	}
	if (operators.empty()) {
		throw std::runtime_error("missing opening parenthesis");
	}
	operators.pop();
}

} // namespace

Fraction::Fraction(i64 num, i64 denom) : numerator(num), denominator(denom) {
    normalize();
}

void Fraction::normalize() {
    if (denominator < 0) {
        denominator = -denominator;
        numerator = -numerator;
    }
    auto d = std::gcd(numerator, denominator);
    numerator /= d;
    denominator /= d;
}

Fraction operator+(const Fraction &a, const Fraction &b) {
    return Fraction{a.numerator * b.denominator + b.numerator * a.denominator,
                    a.denominator * b.denominator};
}

Fraction &Fraction::operator+=(const Fraction &other) {
    *this = *this + other;
    return *this;
}

Fraction Fraction::operator-() const {
    return Fraction(-numerator, denominator);
}

Fraction operator-(const Fraction &a, const Fraction &b) {
    return Fraction{a.numerator * b.denominator - b.numerator * a.denominator,
                    a.denominator * b.denominator};
}

Fraction &Fraction::operator-=(const Fraction &other) {
    *this = *this - other;
    return *this;
}

Fraction operator*(const Fraction &a, const Fraction &b) {
    return Fraction{a.numerator * b.numerator, a.denominator * b.denominator};
}

Fraction &Fraction::operator*=(const Fraction &other) {
    *this = *this * other;
    return *this;
}

Fraction operator/(const Fraction &a, const Fraction &b) {
    if (b.numerator == 0) {
        throw std::runtime_error("division by zero");
    }
    return Fraction{a.numerator * b.denominator, a.denominator * b.numerator};
}

Fraction &Fraction::operator/=(const Fraction &other) {
    *this = *this / other;
    return *this;
}

Fraction operator^(const Fraction &base, int exponent) {
    if (exponent == 0) {
        return Fraction(1, 1);
    }
    if (base.numerator == 0 && exponent < 0) {
        throw std::runtime_error("zero cannot be raised to negative power");
    }
    Fraction result(1, 1);
    Fraction factor = base;
    if (exponent < 0) {
        factor = Fraction(base.denominator, base.numerator);
        exponent = -exponent;
    }
    while (exponent) {
        if (exponent & 1) {
            result *= factor;
        }
        if (exponent > 1) {
            factor *= factor;
        }
        exponent >>= 1;
    }
    return result;
}

Fraction &Fraction::operator^=(int exponent) {
    *this = (*this) ^ exponent;
    return *this;
}

Fraction expression_evaluate(const std::string &expr) {
    std::string input = remove_spaces(expr);
    if (input.empty()) {
        throw std::runtime_error("expression is empty");
    }

    Stack<Fraction> values;
    Stack<char> operators;

    std::size_t index = 0;
    while (index < input.size()) {
        char ch = input[index];

        if (std::isdigit(static_cast<unsigned char>(ch))) {
            Fraction number{parse_integer(input, index), 1};
            values.push(number);
            continue;
        }

        if (ch == '(') {
            operators.push('(');
            ++index;
            continue;
        } else if (ch == ')') {
            ++index;
            collapse(values, operators);
            continue;
        } else {
            if (is_unary(ch, input, index)) {
                int count = 0;
                while (index < input.size() && (input[index] == '+' || input[index] == '-')) {
                    if (input[index] == '-') {
                        ++count;
                    }
                    if (index > 0 && (input[index - 1] == '*' || input[index - 1] == '/' || input[index - 1] == '^')) {
                        throw std::runtime_error("invalid use of unary operator after '*', '/' or '^'");
                    }
                    ++index;
                }
                values.push(Fraction(0, 1));
                ch = (count % 2 == 0) ? '+' : '-';
                --index; // adjust for the upcoming ++index
            }

            process_operator(values, operators, ch);
            ++index;
        }
    }

    while (!operators.empty()) {
        char op = operators.pop();
        if (op == '(' || op == ')') {
            throw std::runtime_error("mismatched parentheses");
        }
        apply_operator(values, op);
    }

    if (values.size() != 1) {
        throw std::runtime_error("malformed expression");
    }
    return values.pop();
}

std::ostream& operator<<(std::ostream &os, const Fraction &value) {
    os << value.numerator << '/' << value.denominator;
    return os;
}