#pragma once

#include "sw_fwd.h"  // Forward declaration

#include "shared.h"
// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;

    template <typename y>
    friend class EnableSharedFromThis;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) {
        block_ = other.block_;
        observer_ = other.observer_;
        IncBlock();
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) {
        block_ = other.block_;
        observer_ = other.observer_;
        IncBlock();
    }

    WeakPtr(WeakPtr&& other) {
        DecBlock();
        block_ = std::move(other.block_);
        observer_ = std::move(other.observer_);
        other.block_ = nullptr;
        other.observer_ = nullptr;
    }

    template <typename Y>
    WeakPtr(WeakPtr<Y>&& other) {
        DecBlock();
        block_ = std::move(other.block_);
        observer_ = std::move(other.observer_);
        other.block_ = nullptr;
        other.observer_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.block_;
        if (block_) {
            observer_ = other.observer_;
            IncBlock();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (other.block_) {
            other.block_->IncWeakCounter();
        }
        DecBlock();
        block_ = other.block_;
        observer_ = other.observer_;
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        if (other.block_) {
            other.block_->IncWeakCounter();
        }
        DecBlock();
        block_ = other.block_;
        observer_ = other.observer_;
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        DecBlock();
        block_ = std::move(other.block_);
        observer_ = std::move(other.observer_);
        other.block_ = nullptr;
        other.observer_ = nullptr;
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        DecBlock();
        block_ = std::move(other.block_);
        observer_ = std::move(other.observer_);
        other.block_ = nullptr;
        other.observer_ = nullptr;
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const SharedPtr<Y>& other) {
        DecBlock();
        block_ = other.block_;
        if (block_) {
            block_->IncWeakCounter();
        }
        observer_ = other.observer_;
        return *this;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        DecBlock();
        observer_ = nullptr;
        block_ = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        // observer_ = nullptr;
        DecBlock();
        block_ = nullptr;
        observer_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(observer_, other.observer_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_) {
            return block_->GetStrongCounter();
        }
        return 0;
    }
    bool Expired() const {
        if (UseCount() == 0) {
            return true;
        }
        return false;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>();
        } else {
            return SharedPtr<T>(*this);
        }
    }

private:
    T* observer_ = nullptr;
    BaseBlock* block_ = nullptr;
    void IncBlock() {
        if (block_) {
            block_->IncWeakCounter();
        }
    }
    void DecBlock() {
        if (block_) {
            block_->DecWeakCounter();
            if (block_->GetStrongCounter() == 0 && block_->GetWeakCounter() == 0) {
                delete block_;
                block_ = nullptr;
            }
        }
        // надо что то еще?
    }
};
