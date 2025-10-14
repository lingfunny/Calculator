#include "polynomial.hpp"

#include <format>
#include <iostream>

namespace {

constexpr double EPSILON = 1e-9;

bool is_zero(double value) {
	return std::abs(value) < EPSILON;
}

PolyTerm *copy_terms(const PolyTerm *source) {
	// copy all terms after source (inclusive)
	if (!source) {
		return nullptr;
	}
	PolyTerm *head = nullptr;
	PolyTerm **tail = &head;
	while (source) {
		*tail = new PolyTerm{source->coefficient, source->exponent, nullptr};
		tail = &((*tail)->next);
		source = source->next;
	}
	return head;
}

void insert_term(PolyTerm *&head, double coefficient, int exponent) {
	// insert term to the list
	// while keeping the list sorted by exponent in descending order
	if (is_zero(coefficient)) {
		return;
	}

	PolyTerm **current = &head;	// current points to the pointer to the current node
	while (*current && (*current)->exponent > exponent) {
		current = &((*current)->next);
	}

	if (*current && (*current)->exponent == exponent) {
		(*current)->coefficient += coefficient;
		if (is_zero((*current)->coefficient)) {
			PolyTerm *to_delete = *current;
			*current = (*current)->next;
			delete to_delete;
		}
		return;
	}

	PolyTerm *node = new PolyTerm{coefficient, exponent, *current};
	*current = node;
}

std::size_t count_terms(const PolyTerm *node) {
	std::size_t count = 0;
	while (node) {
		++count;
		node = node->next;
	}
	return count;
}

void delete_terms(PolyTerm *node) {
	// delete all terms after node (inclusive)
	while (node) {
		PolyTerm *next = node->next;
		delete node;
		node = next;
	}
}

double power(double base, int exp) {
	if (exp < 0) {
		base = 1.0 / base;
		exp = -exp;
	}
	double result = 1.0;
	while (exp) {
		if (exp & 1) result *= base;
		base *= base;
		exp >>= 1;
	}
	return result;
}

} // namespace

Polynomial::Polynomial() : head(nullptr) {}

Polynomial::Polynomial(const Polynomial &other) : head(copy_terms(other.head)) {}

Polynomial::Polynomial(Polynomial &&other) noexcept : head(other.head) {
	other.head = nullptr;
}

Polynomial::~Polynomial() {
	delete_terms(head);
	head = nullptr;
}

Polynomial &Polynomial::operator=(Polynomial other) noexcept {
	swap(*this, other);
	return *this;
}

void swap(Polynomial &a, Polynomial &b) noexcept {
	auto tmp = a.head;
	a.head = b.head;
	b.head = tmp;
}

Polynomial &Polynomial::operator+=(const Polynomial &other) {
	const PolyTerm *node = other.head;
	while (node) {
		insert_term(head, node->coefficient, node->exponent);
		node = node->next;
	}
	return *this;
}

Polynomial Polynomial::operator-() const {
	Polynomial result;
	const PolyTerm *node = head;
	while (node) {
		insert_term(result.head, -node->coefficient, node->exponent);
		node = node->next;
	}
	return result;
}

Polynomial &Polynomial::operator-=(const Polynomial &other) {
	return *this += (-other);
}

Polynomial &Polynomial::operator*=(const Polynomial &other) {
	Polynomial temp = (*this) * other;
	swap(*this, temp);
	return *this;
}

double Polynomial::evaluate(double x) const {
	double result = 0.0;
	const PolyTerm *node = head;
	while (node) {
		result += node->coefficient * power(x, node->exponent);
		node = node->next;
	}
	return result;
}

Polynomial Polynomial::derivative() const {
	Polynomial result;
	const PolyTerm *node = head;
	while (node) {
		if (node->exponent != 0) {
			insert_term(result.head, node->coefficient * node->exponent, node->exponent - 1);
		}
		node = node->next;
	}
	return result;
}

void Polynomial::print() const {
	if (!head) {
		std::cout << 0 << std::endl;
		return;
	}

	std::size_t term_count = count_terms(head);
	std::cout << term_count;
	const PolyTerm *node = head;
	while (node) {
		std::cout << ' ' << node->coefficient << ' ' << node->exponent;
		node = node->next;
	}
	std::cout << std::endl;
}

void Polynomial::printLaTeX() const {
	if (!head) {
		std::cout << "$0$" << std::endl;
		return;
	}

	std::cout << "$";

	const PolyTerm *node = head;
	bool first = true;
	while (node) {
		double coeff = node->coefficient;
		int exponent = node->exponent;

		if (first) {
			if (coeff < 0) {
				std::cout << "-";
			}
		} else {
			std::cout << (coeff < 0 ? " - " : " + ");
		}

		double abs_coeff = std::abs(coeff);
		bool omit_coeff = is_zero(abs_coeff - 1.0) && exponent != 0;
		if (!omit_coeff || exponent == 0) {
			std::cout << std::format("{:g}", abs_coeff);
		}

		if (exponent != 0) {
			std::cout << "x";
			if (exponent != 1) {
				std::cout << std::format("^{{{}}}", exponent);
			}
		}

		first = false;
		node = node->next;
	}
	std::cout << "$" << std::endl;
}

Polynomial operator+(const Polynomial &a, const Polynomial &b) {
	Polynomial result = a;
	return result += b;
}

Polynomial operator-(const Polynomial &a, const Polynomial &b) {
	Polynomial result = a;
	return result -= b;
}

Polynomial operator*(const Polynomial &a, const Polynomial &b) {
	Polynomial result;
	for (const PolyTerm *pa = a.head; pa; pa = pa->next) {
		for (const PolyTerm *pb = b.head; pb; pb = pb->next) {
			insert_term(result.head, pa->coefficient * pb->coefficient, pa->exponent + pb->exponent);
		}
	}
	return result;
}

void Polynomial::addTerm(double coefficient, int exponent) {
	insert_term(head, coefficient, exponent);
}

Polynomial createPoly() {
	Polynomial p;
	int n;
	std::cin >> n;
	while (n--) {
		double coeff;
		int exp;
		std::cin >> coeff >> exp;
		p.addTerm(coeff, exp);
	}
	return p;
}