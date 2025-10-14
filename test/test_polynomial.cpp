#include <iostream>
#include "polynomial.hpp"

int main() {
    freopen("polynomial.in", "r", stdin);
    freopen("polynomial.out", "w", stdout);

    int T;
    std::cin >> T;
    while (T--) {
        Polynomial p1 = createPoly();
        Polynomial p2 = createPoly();

        char op;
        std::cin >> op;

        if (op == '+') {
            (p1 + p2).printLaTeX();
        } else if (op == '-') {
            (p1 - p2).printLaTeX();
        } else if (op == '*') {
            (p1 * p2).printLaTeX();
        } else if (op == 'e') {
            double x;
            std::cin >> x;
            std::cout << p1.evaluate(x) << std::endl;
        } else if (op == 'd') {
            p1.derivative().printLaTeX();
        }
    }
}