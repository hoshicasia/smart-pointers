#pragma once

#include <exception>

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

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;


class EnableSharedFromThisBase {

};
