#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    size_t IncRef() {
        count_++;
        return count_;
    }
    size_t DecRef() {
        count_--;
        return count_;
    }
    size_t RefCount() const {
        return count_;
    }
    SimpleCounter& operator=(const SimpleCounter& other) {
        return *this;
    }

private:
    size_t count_ = 0;
};

struct DefaultDelete {
    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    void IncRef() {
        counter_.IncRef();
    }

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        counter_.DecRef();
        if (counter_.RefCount() == 0) {
            Deleter().Destroy(static_cast<Derived*>(this));
        }
    }

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    }

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() = default;
    IntrusivePtr(std::nullptr_t) {
        ptr_ = nullptr;
    }
    IntrusivePtr(T* ptr) {
        ptr_ = ptr;
        if (ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) {
        ptr_ = other.ptr_;
        if (other.ptr_) {
            ptr_->IncRef();
        }
    }

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) {
        Dec();
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    IntrusivePtr(const IntrusivePtr& other) {
        ptr_ = other.ptr_;
        if (other.ptr_) {
            ptr_->IncRef();
        }
    }

    IntrusivePtr(IntrusivePtr&& other) {
        Dec();
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
    }

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (other.ptr_) {
            other.ptr_->IncRef();
        }
        Dec();
        ptr_ = other.ptr_;
        return *this;
    }
    template <typename Y>
    IntrusivePtr& operator=(const IntrusivePtr<Y>& other) {
        if (other.ptr_) {
            other.ptr_->IncRef();
        }
        Dec();
        ptr_ = other.ptr_;
        return *this;
    }

    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (ptr_ != other.ptr_) {
            Dec();
            ptr_ = std::move(other.ptr_);
            other.ptr_ = nullptr;
        }
        return *this;
    }

    template <typename Y>
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (ptr_ != other.ptr_) {
            Dec();
            ptr_ = std::move(other.ptr_);
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // Destructor
    ~IntrusivePtr() {
        Dec();
        ptr_ = nullptr;
    }

    // Modifiers
    void Reset() {
        Dec();
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        Dec();
        if (ptr) {
            ptr->IncRef();
        }
        if (ptr_ != ptr) {
            ptr_ = ptr;
        }
    }
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    // Observers
    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *Get();
    }
    T* operator->() const {
        return Get();
    }
    size_t UseCount() const {
        if (ptr_) {
            return ptr_->RefCount();
        }
        return 0;
    }
    explicit operator bool() const {
        if (ptr_) {
            return true;
        }
        return false;
    }

private:
    void Dec() {
        if (ptr_) {
            ptr_->DecRef();
        }
    }
    T* ptr_ = nullptr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    IntrusivePtr<T> new_iptr = IntrusivePtr(new T(std::forward<Args>(args)...));
    return new_iptr;
}
