#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <typename T>
class DefaultDelete {
public:
    DefaultDelete() {
    }
    void operator()(T* ptr) {
        delete ptr;
    }
    template <class U>
    DefaultDelete(const DefaultDelete<U>&) {
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDelete<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : inner_(ptr, Deleter()){};
    UniquePtr(T* ptr, Deleter&& deleter) : inner_(ptr, std::forward<decltype(deleter)>(deleter)){};
    UniquePtr(T* ptr, const Deleter& deleter)
        : inner_(ptr, std::forward<decltype(deleter)>(deleter)){};
    template <typename U, typename Del>
    UniquePtr(UniquePtr<U, Del>&& other) noexcept {
        GetInner() = other.Release();
        inner_.GetSecond() = std::move(other.GetInnerDeleter());
    };

    UniquePtr(const UniquePtr&) = delete;

    // `operator=`-s
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::move(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (Get()) {
            GetDeleter()(GetInner());
        }
        GetInner() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* tmp = Get();
        inner_.GetFirst() = nullptr;
        return tmp;
        // что то с Deleter??
    }
    void Reset(T* ptr = nullptr) {
        T* old_ptr = Get();
        inner_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(GetInner(), other.GetInner());
        std::swap(GetDeleter(), other.GetDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return inner_.GetFirst();
    }
    Deleter& GetDeleter() {
        return inner_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return inner_.GetSecond();
    }
    explicit operator bool() const {
        return Get();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *Get();
    }
    T* operator->() const {
        return Get();
    }
    T*& GetInner() {
        return inner_.GetFirst();
    }
    Deleter& GetInnerDeleter() {
        return inner_.GetSecond();
    }

private:
    CompressedPair<T*, Deleter> inner_;
};

template <typename T>
class DefaultDelete<T[]> {
public:
    DefaultDelete() {
    }
    void operator()(T* ptr) {
        delete[] ptr;
    }
    template <class U>
    DefaultDelete(const DefaultDelete<U[]>&) {
    }
};
// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : inner_(ptr, Deleter()){};
    UniquePtr(T* ptr, Deleter&& deleter) : inner_(ptr, std::forward<decltype(deleter)>(deleter)){};
    UniquePtr(T* ptr, const Deleter& deleter)
        : inner_(ptr, std::forward<decltype(deleter)>(deleter)){};
    UniquePtr(UniquePtr&& other) noexcept {
        inner_.GetFirst() = other.Release();
        inner_.GetSecond() = std::move(other.GetDeleter());
    }
    UniquePtr(const UniquePtr&) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        GetDeleter() = std::move(other.GetDeleter());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (Get()) {
            GetDeleter()(GetInner());
        }
        GetInner() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* tmp = Get();
        inner_.GetFirst() = nullptr;
        return tmp;
        // что то с Deleter??
    }
    void Reset(T* ptr = nullptr) {
        T* old_ptr = Get();
        inner_.GetFirst() = ptr;
        if (old_ptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        std::swap(GetInner(), other.GetInner());
        std::swap(GetDeleter(), other.GetDeleter());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return inner_.GetFirst();
    }
    Deleter& GetDeleter() {
        return inner_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return inner_.GetSecond();
    }
    explicit operator bool() const {
        return Get();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator*() const {
        return *Get();
    }
    T* operator->() const {
        return Get();
    }
    T& operator[](std::size_t i) const {
        return Get()[i];
    }
    T*& GetInner() {
        return inner_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> inner_;
};