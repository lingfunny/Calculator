#include "stack.hpp"

#include "expression.hpp"

#include <stdexcept>

namespace {

template <typename T>
T *allocate(std::size_t capacity) {
    if (capacity == 0) {
        return nullptr;
    }
    return new T[capacity];
}

} // namespace

template <typename T>
Stack<T>::Stack() : data_(nullptr), size_(0), capacity_(DEFAULT_CAPACITY) {
    data_ = allocate<T>(capacity_);
}

template <typename T>
Stack<T>::Stack(std::size_t initial_capacity)
    : data_(nullptr), size_(0), capacity_(initial_capacity == 0 ? DEFAULT_CAPACITY : initial_capacity) {
    data_ = allocate<T>(capacity_);
}

template <typename T>
Stack<T>::Stack(const Stack &other) : data_(nullptr), size_(other.size_), capacity_(other.capacity_) {
    // copy constructor
    data_ = allocate<T>(capacity_);
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
}

template <typename T>
Stack<T>::Stack(Stack &&other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
    // move constructor
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

template <typename T>
Stack<T>::~Stack() {
    delete[] data_;
    data_ = nullptr;
    size_ = 0;
    capacity_ = 0;
}

template <typename T>
Stack<T> &Stack<T>::operator=(const Stack &other) {
    // copy
    if (this == &other) {
        return *this;
    }
    if (other.size_ > capacity_) {
        delete[] data_;
        capacity_ = other.capacity_;
        if (capacity_ < other.size_) {
            capacity_ = other.size_;
        }
        data_ = allocate<T>(capacity_);
    }
    size_ = other.size_;
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
    return *this;
}

template <typename T>
Stack<T> &Stack<T>::operator=(Stack &&other) noexcept {
    // move
    if (this == &other) {
        return *this;
    }
    delete[] data_;
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
    return *this;
}

template <typename T>
void Stack<T>::ensure_capacity(std::size_t min_capacity) {
    if (capacity_ >= min_capacity) {
        return;
    }
    std::size_t new_capacity = capacity_ == 0 ? DEFAULT_CAPACITY : capacity_;
    while (new_capacity < min_capacity) {
        new_capacity *= 2;
    }
    T *new_data = allocate<T>(new_capacity);
    for (std::size_t i = 0; i < size_; ++i) {
        new_data[i] = data_[i];
    }
    delete[] data_;
    data_ = new_data;
    capacity_ = new_capacity;
}

template <typename T>
void Stack<T>::push(const T &value) {
    ensure_capacity(size_ + 1);
    data_[size_++] = value;
}

template <typename T>
T Stack<T>::pop() {
    if (size_ == 0) {
        throw std::underflow_error("stack underflow");
    }
    return data_[--size_];
}

template <typename T>
const T &Stack<T>::top() const {
    if (size_ == 0) {
        throw std::underflow_error("stack is empty");
    }
    return data_[size_ - 1];
}

template <typename T>
bool Stack<T>::empty() const {
    return size_ == 0;
}

template <typename T>
std::size_t Stack<T>::size() const {
    return size_;
}

template <typename T>
void Stack<T>::clear() {
    size_ = 0;
}

template class Stack<char>;
template class Stack<Fraction>;