#pragma once

#include <cstddef>

template <typename T>
class Stack {
public:
    Stack();
    explicit Stack(std::size_t initial_capacity); // initialize with given capacity
    Stack(const Stack &other); // copy constructor
    Stack(Stack &&other) noexcept; // move constructor
    ~Stack();

    Stack &operator=(const Stack &other);
    Stack &operator=(Stack &&other) noexcept;

    void push(const T &value);
    T pop();
    const T &top() const;
    bool empty() const;
    std::size_t size() const;
    void clear();

private:
    static constexpr std::size_t DEFAULT_CAPACITY = 8;

    T *data_;
    std::size_t size_;
    std::size_t capacity_;

    void ensure_capacity(std::size_t min_capacity);
};
