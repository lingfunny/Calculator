#pragma once

struct PolyTerm {
	double coefficient;
	int exponent;
	PolyTerm *next;
};

struct Polynomial {
	Polynomial();
	Polynomial(const Polynomial& other);
	Polynomial(Polynomial&& other) noexcept;
	~Polynomial();

	Polynomial& operator=(Polynomial other) noexcept; // copy-swap
	friend void swap(Polynomial& a, Polynomial& b) noexcept;

	friend Polynomial operator+(const Polynomial &a, const Polynomial &b);
	Polynomial& operator+=(const Polynomial &other);
	Polynomial operator-() const;
	friend Polynomial operator-(const Polynomial &a, const Polynomial &b);
	Polynomial& operator-= (const Polynomial &other);
	friend Polynomial operator*(const Polynomial &a, const Polynomial &b);
	Polynomial& operator*=(const Polynomial &other);

	double evaluate(double x) const;
	Polynomial derivative() const;
	void addTerm(double coefficient, int exponent);

	void print() const;
	void printLaTeX() const;

	private:
	PolyTerm *head;
	// PolyTerms in descending order of exponent
};

Polynomial createPoly();