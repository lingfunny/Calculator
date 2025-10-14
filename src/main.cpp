#include "expression.hpp"
#include "polynomial.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

struct CLIContext {
    std::unordered_map<std::string, Polynomial> polynomials;
};

#ifdef _WIN32
void enable_virtual_terminal_processing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return;
    }
    DWORD consoleMode = 0;
    if (!GetConsoleMode(hOut, &consoleMode)) {
        return;
    }
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, consoleMode);
}
#endif

std::string trim(std::string_view text) {
    const auto begin = text.find_first_not_of(" \t\n\r");
    if (begin == std::string_view::npos) {
        return {};
    }
    const auto end = text.find_last_not_of(" \t\n\r");
    return std::string{text.substr(begin, end - begin + 1)};
}

std::pair<std::string, std::string> split_command(const std::string &line) {
    std::string trimmed = trim(line);
    if (trimmed.empty()) {
        return {"", ""};
    }
    auto pos = trimmed.find_first_of(" \t");
    if (pos == std::string::npos) {
        std::string cmd = trimmed;
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return {cmd, ""};
    }
    std::string cmd = trimmed.substr(0, pos);
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    std::string rest = trim(trimmed.substr(pos + 1));
    return {cmd, rest};
}

void print_banner() {
    const char *banner =
        "\n"
        "╔════════════════════════════════════════════════╗\n"
        "║            Polynomial & Expression             ║\n"
        "║                Calculator CLI                  ║\n"
        "╚════════════════════════════════════════════════╝\n";
    std::cout << banner;
}

void print_help() {
    constexpr int COL_WIDTH = 28;
    std::cout << "可用命令：\n";
    std::cout << std::left
              << std::setw(COL_WIDTH) << "  help" << "显示帮助" << '\n'
              << std::setw(COL_WIDTH) << "  expr <expression>" << "计算分式四则表达式" << '\n'
              << std::setw(COL_WIDTH) << "  poly new <name>" << "交互式创建多项式" << '\n'
              << std::setw(COL_WIDTH) << "  poly list" << "列出已保存的多项式" << '\n'
              << std::setw(COL_WIDTH) << "  poly show <name>" << "显示多项式" << '\n'
              << std::setw(COL_WIDTH) << "  poly eval <name> <x>" << "计算 P(x)" << '\n'
              << std::setw(COL_WIDTH) << "  poly deriv <name>" << "输出导数" << '\n'
              << std::setw(COL_WIDTH) << "  poly add <A> <B>" << "显示 A+B 的结果" << '\n'
              << std::setw(COL_WIDTH) << "  poly sub <A> <B>" << "显示 A-B 的结果" << '\n'
              << std::setw(COL_WIDTH) << "  poly mul <A> <B>" << "显示 A×B 的结果" << '\n'
              << std::setw(COL_WIDTH) << "  exit" << "退出程序" << '\n';
}

void print_fraction(const Fraction &value) {
    long double approx = static_cast<long double>(value.numerator) / static_cast<long double>(value.denominator);
    std::cout << std::format("结果 = {}/{}   (≈ {:.15g})\n", value.numerator, value.denominator, approx);
}

Polynomial &require_polynomial(CLIContext &ctx, const std::string &name) {
    auto it = ctx.polynomials.find(name);
    if (it == ctx.polynomials.end()) {
        throw std::runtime_error(std::format("未找到名为 '{}' 的多项式", name));
    }
    return it->second;
}

void handle_expr_command(const std::string &payload) {
    std::string expr = trim(payload);
    if (expr.empty()) {
        throw std::runtime_error("用法：expr <expression>");
    }
    Fraction result = expression_evaluate(expr);
    print_fraction(result);
}

void handle_poly_new(CLIContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("用法：poly new <name>");
    }
    std::string name = args[1];
    std::cout << "输入项数量以及各项 (系数 指数)，例如：\n";
    std::cout << "3  2 2  -1 1  5 0\n表示 3 个项：2x^2 - 1x + 5\n> ";
    Polynomial poly = createPoly();
    ctx.polynomials[name] = std::move(poly);
    std::cout << std::format("多项式 '{}' 已保存。\n", name);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void handle_poly_list(const CLIContext &ctx) {
    if (ctx.polynomials.empty()) {
        std::cout << "尚未保存任何多项式。\n";
        return;
    }
    std::cout << "已保存的多项式：\n";
    for (const auto &entry : ctx.polynomials) {
        std::cout << "  • " << entry.first << '\n';
    }
}

void handle_poly_show(CLIContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("用法：poly show <name> [-l, --latex]");
    }
    Polynomial &poly = require_polynomial(ctx, args[1]);
    if (args.size() >= 3 && (args[2] == "-l" || args[2] == "--latex")) {
        std::cout << "  LaTeX 格式：";
        poly.printLaTeX();
    } else {
        std::cout << "  表达式：";
        poly.print();
    }
}

void handle_poly_eval(CLIContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 3) {
        throw std::runtime_error("用法：poly eval <name> <x>");
    }
    Polynomial &poly = require_polynomial(ctx, args[1]);
    double x;
    try {
        x = std::stod(args[2]);
    } catch (const std::exception &) {
        throw std::runtime_error("x 必须是数字");
    }
    double value = poly.evaluate(x);
    std::cout << std::format("P({}) = {:.10g}\n", x, value);
}

void handle_poly_deriv(CLIContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 2) {
        throw std::runtime_error("用法：poly deriv <name> [-l, --latex]");
    }
    Polynomial &poly = require_polynomial(ctx, args[1]);
    Polynomial deriv = poly.derivative();
    if (args.size() >= 3 && (args[2] == "-l" || args[2] == "--latex")) {
        std::cout << "  LaTeX 格式：";
        deriv.printLaTeX();
    } else {
        std::cout << "  表达式：";
        deriv.print();
    }
}

Polynomial calculate_binary(const Polynomial &lhs, const Polynomial &rhs, const std::string &op) {
    if (op == "add") {
        return lhs + rhs;
    }
    if (op == "sub") {
        return lhs - rhs;
    }
    if (op == "mul") {
        return lhs * rhs;
    }
    throw std::runtime_error("不支持的运算");
}

void handle_poly_binary(CLIContext &ctx, const std::vector<std::string> &args, const std::string &op) {
    if (args.size() < 3) {
        throw std::runtime_error(std::format("用法：poly {} <A> <B> [-l, --latex]", op));
    }
    const Polynomial &lhs = require_polynomial(ctx, args[1]);
    const Polynomial &rhs = require_polynomial(ctx, args[2]);
    Polynomial result = calculate_binary(lhs, rhs, op);
    std::cout << std::format("{}({}, {}) = ", op, args[1], args[2]);
    if (args.size() >= 4 && (args[3] == "-l" || args[3] == "--latex")) {
        result.printLaTeX();
    } else {
        result.print();
    }
}

void split_args(const std::string &payload, std::vector<std::string> &args) {
    std::istringstream iss(payload);
    std::string token;
    while (iss >> token) {
        args.push_back(token);
    }
}

void handle_poly_command(CLIContext &ctx, const std::string &payload) {
    std::vector<std::string> args;
    split_args(payload, args);
    if (args.empty()) {
        throw std::runtime_error("用法：poly <subcommand> ...，输入 help 查看详情");
    }
    std::string sub = args[0];
    std::transform(sub.begin(), sub.end(), sub.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (sub == "new") {
        handle_poly_new(ctx, args);
    } else if (sub == "list") {
        handle_poly_list(ctx);
    } else if (sub == "show") {
        handle_poly_show(ctx, args);
    } else if (sub == "eval") {
        handle_poly_eval(ctx, args);
    } else if (sub == "deriv" || sub == "diff") {
        handle_poly_deriv(ctx, args);
    } else if (sub == "add" || sub == "sub" || sub == "mul") {
        handle_poly_binary(ctx, args, sub);
    } else {
        throw std::runtime_error(std::format("未知的 poly 子命令：{}", sub));
    }
}

} // namespace

int main() {
#ifdef _WIN32
    enable_virtual_terminal_processing();
#endif
    CLIContext context;
    print_banner();
    print_help();

    std::string line;
    while (std::cout << "\n> " && std::getline(std::cin, line)) {
        auto [command, payload] = split_command(line);
        if (command.empty()) {
            continue;
        }
        try {
            if (command == "exit" || command == "quit") {
                std::cout << "再见！\n";
                break;
            }
            if (command == "help") {
                print_help();
                continue;
            }
            if (command == "expr") {
                handle_expr_command(payload);
                continue;
            }
            if (command == "poly") {
                handle_poly_command(context, payload);
                continue;
            }
            if (command == "banner") {
                print_banner();
                continue;
            }
            std::cout << std::format("未知指令：{}，输入 help 查看帮助。\n", command);
        } catch (const std::exception &e) {
            std::cout << std::format("错误：{}\n", e.what());
        }
    }
    return 0;
}