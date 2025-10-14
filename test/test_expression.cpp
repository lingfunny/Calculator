#include <iostream>
#include "expression.hpp"

int main() {
    freopen("expression.in", "r", stdin);
    char s[10000];
    std::cin.getline(s, 10000);
    int T = atoi(s);
    while (T--) {
        try {
            std::cin.getline(s, 10000);
            expression_evaluate(s).println();
        } catch (const std::exception &e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
}