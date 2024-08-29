#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
class BaseBlock {
protected:
    size_t strong_counter_ = 1;
    size_t weak_counter_ = 0;

public:
    BaseBlock() = default;
    virtual void IncStrongCounter() {
        strong_counter_++;
    }
    virtual void IncWeakCounter() {
        weak_counter_++;
    }
    virtual void DecStrongCounter() = 0;
    virtual void DecWeakCounter() {
        weak_counter_--;
    }
    // virtual T* GetObjPtr() = 0;
    virtual ~BaseBlock() {
    }

    size_t GetStrongCounter() {
        return strong_counter_;
    }
    size_t GetWeakCounter() {
        return weak_counter_;
    }
};

template <typename T>
class CBlockObj : public BaseBlock {
protected:
    std::aligned_storage_t<sizeof(T), alignof(T)> obj_;
    // alignas(T) char obj_[sizeof(T)];
    //  align storage?? <sizeof(T), align(T)>
    //  new (ptr T) (T(std::forward))
    //  to delete: reinterpret_cast<T*>(obj_)->~T()
    /*std::aligned_storage_t<sizeof(T), alignof(T)> buf;
        unsigned char* storage = reinterpret_cast<unsigned char*>(&buf);
      */

public:
    template <typename... Args>
    CBlockObj(Args&&... args) {
        ::new (reinterpret_cast<T*>(&obj_)) T(std::forward<Args>(args)...);
    }
    void DecStrongCounter() override {
        strong_counter_ -= 1;
        if (!strong_counter_) {
            reinterpret_cast<T*>(&obj_)->~T();
        }
    }

    ~CBlockObj() override {
        // reinterpret_cast<T*>(obj_)->~T();
    }

    T* GetObject() {
        return reinterpret_cast<T*>(&obj_);
    }
};

template <typename T>
class CBlockPtr : public BaseBlock {
protected:
    T* obj_ptr_ = nullptr;

public:
    CBlockPtr(T* ptr) {
        obj_ptr_ = ptr;
    }
    void DecStrongCounter() override {
        strong_counter_ -= 1;
        if (!strong_counter_) {
            delete obj_ptr_;
            obj_ptr_ = nullptr;
        }
    }

    ~CBlockPtr() override {
        if (obj_ptr_) {
            delete obj_ptr_;
            obj_ptr_ = nullptr;
        }
    }
};

class EnableSharedFromThisBase {};

template <typename T>
class EnableSharedFromThis;

template <typename T>
class SharedPtr {
    template <typename Y>
    friend class SharedPtr;

    template <typename Y, typename... Args>
    friend SharedPtr<Y> MakeShared(Args&&... args);

    template <typename Y>
    friend class WeakPtr;

    template <typename y>
    friend class EnableSharedFromThis;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() = default;
    SharedPtr(std::nullptr_t) : observer_(nullptr), block_(nullptr) {
    }
    explicit SharedPtr(T* ptr) : observer_(ptr) {
        block_ = new CBlockPtr<T>(ptr);
        if constexpr (std::is_convertible<T*, EnableSharedFromThisBase*>::value) {
            InitWeakThis(ptr);
        }
    }

    template <typename Y>
    explicit SharedPtr(Y* ptr) : observer_(ptr) {
        block_ = new CBlockPtr<Y>(ptr);
        if constexpr (std::is_convertible<T*, EnableSharedFromThisBase*>::value) {
            InitWeakThis(ptr);
        }
    }

    SharedPtr(const SharedPtr& other) {
        observer_ = other.observer_;
        block_ = other.block_;
        IncBlock();
    }
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        DecBlock();
        observer_ = other.observer_;
        block_ = other.block_;
        IncBlock();
    }
    SharedPtr(SharedPtr&& other) {
        DecBlock();
        observer_ = other.observer_;
        block_ = other.block_;
        other.block_ = nullptr;
        other.observer_ = nullptr;
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) {
        DecBlock();
        observer_ = other.observer_;
        block_ = other.block_;
        other.block_ = nullptr;
        other.observer_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        observer_ = ptr;
        block_ = other.block_;
        IncBlock();
    }

    // Promote WeakPtr
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.block_ && other.block_->GetStrongCounter() == 0 &&
            other.block_->GetWeakCounter() != 0) {
            throw BadWeakPtr();
        }
        block_ = other.block_;
        if (block_) {
            observer_ = other.observer_;
            IncBlock();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // operator=-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (other.block_) {
            other.block_->IncStrongCounter();
        }
        DecBlock();
        block_ = other.block_;
        observer_ = other.observer_;
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        if (other.block_) {
            other.block_->IncStrongCounter();
        }
        DecBlock();
        block_ = other.block_;
        observer_ = other.observer_;
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        DecBlock();
        block_ = std::move(other.block_);
        observer_ = std::move(other.observer_);
        other.block_ = nullptr;
        other.observer_ = nullptr;
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        DecBlock();
        block_ = std::move(other.block_);
        observer_ = std::move(other.observer_);
        other.block_ = nullptr;
        other.observer_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        DecBlock();
        observer_ = nullptr;
        block_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers
    void Reset() {
        observer_ = nullptr;
        DecBlock();
        block_ = nullptr;
    }
    template <typename Y>
    void Reset(Y* ptr) {
        DecBlock();
        observer_ = ptr;
        if (ptr) {
            block_ = new CBlockPtr<Y>(ptr);
        }
    }

    void Swap(SharedPtr& other) {
        std::swap(other.block_, block_);
        std::swap(observer_, other.observer_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers
    T* Get() const {
        return observer_;
    }
    T& operator*() const {
        return *Get();
    }
    T* operator->() const {
        return Get();
    }
    size_t UseCount() const {
        if (block_) {
            return block_->GetStrongCounter();
        }
        return 0;
    }
    explicit operator bool() const {
        if (observer_) {
            return true;
        }
        return false;
    }

    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        e->weak_this_ = *this;
    }

private:
    // поменять??
    void DecBlock() {
        if (block_) {
            if (block_->GetStrongCounter() == 1) {
                block_->IncWeakCounter();
                block_->DecStrongCounter();
                block_->DecWeakCounter();
            } else {
                block_->DecStrongCounter();
            }
            if (block_ && UseCount() == 0 && block_->GetWeakCounter() == 0) {
                delete block_;
                block_ = nullptr;
            }
        }
    }
    void IncBlock() {
        if (block_) {
            block_->IncStrongCounter();
        }
    }
    BaseBlock* block_ = nullptr;
    T* observer_ = nullptr;
};

template <typename T, typename Y>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<Y>& right) {
    return (left.Get() == right.Get());
}

template <class T>
bool operator==(const SharedPtr<T>& left, std::nullptr_t) noexcept {
    return !left;
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    SharedPtr<T> new_sptr;
    CBlockObj<T>* b = new CBlockObj<T>(std::forward<Args>(args)...);
    new_sptr.observer_ = b->GetObject();
    new_sptr.block_ = b;
    if constexpr (std::is_convertible<T*, EnableSharedFromThisBase*>::value) {
        new_sptr.InitWeakThis(new_sptr.observer_);
    }
    return new_sptr;
}

// Look for usage examples in tests

template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
    template <typename Y>
    friend class SharedPtr;

    template <typename Y, typename... Args>
    friend SharedPtr<Y> MakeShared(Args&&... args);

    template <typename Y>
    friend class WeakPtr;

public:
    EnableSharedFromThis() noexcept = default;
    EnableSharedFromThis(const EnableSharedFromThis& other) {
        weak_this_ = other.weak_this_;
    }
    ~EnableSharedFromThis() = default;
    SharedPtr<const T> SharedFromThis() const {
        return SharedPtr<T>(weak_this_);
    }

    SharedPtr<T> SharedFromThis() {
        return SharedPtr<T>(weak_this_);
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_this_;
    }

private:
    WeakPtr<T> weak_this_;
};