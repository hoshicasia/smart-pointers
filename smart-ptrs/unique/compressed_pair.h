#pragma once

#include <type_traits>
#include <utility>
// Me think, why waste time write lot code, when few code do trick.
/*
template <typename F>
concept empty = std::is_empty_v<F>;

template <typename F>
concept not_empty = !std::is_empty_v<F>;

template <typename F, typename S>
concept same = std::is_same_v<F, S>;

template <typename F, typename S>
concept not_same = !std::is_same_v<F, S>;
*/
template <typename F, typename S, bool = std::is_empty_v<F> && !std::is_final_v<F>,
          bool = std::is_empty_v<S> && !std::is_final_v<S>,
          bool = std::is_base_of_v<F, S> || std::is_base_of_v<S, F>>
class CompressedPair;

template <typename F, typename S>
class CompressedPair<F, S, true, true, false> : F, S {
public:
    CompressedPair() = default;
    CompressedPair(const F& first, const S& second) : F(first), S(second) {
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)), S(second) {
    }
    CompressedPair(const F& first, S&& second) : F(first), S(std::move(second)) {
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)), S(std::move(second)) {
    }

    F& GetFirst() {
        return *this;
    }

    S& GetSecond() {
        return *this;
    }

    const F& GetFirst() const {
        return *this;
    };

    const S& GetSecond() const {
        return *this;
    };

private:
};

template <typename F, typename S>
class CompressedPair<F, S, true, true, true> : S {
public:
    CompressedPair() : second_() {
    }
    CompressedPair(const F& first, const S& second) : F(first) {
        second_ = second;
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)) {
        second_ = second;
    }
    CompressedPair(const F& first, S&& second) : F(first) {
        second_ = std::move(second);
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)) {
        second_ = std::move(second);
    }

    F& GetFirst() {
        return *this;
    }
    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return *this;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, false, false> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(const F& first, const S& second) {
        first_ = first;
        second_ = second;
    }
    CompressedPair(F&& first, const S& second) {
        first_ = std::move(first);
        second_ = second;
    }
    CompressedPair(const F& first, S&& second) {
        first_ = first;
        second_ = std::move(second);
    }
    /*CompressedPair(F&& first, S&& second) {
        first_ = std::move(first);
        second_ = std::move(second);
    }*/
    CompressedPair(F&& first, S&& second) : first_(std::move(first)), second_(std::move(second)) {
    }

    F& GetFirst() {
        return first_;
    }
    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, false, true> {
public:
    CompressedPair() : first_(), second_() {
    }
    CompressedPair(const F& first, const S& second) {
        first_ = first;
        second_ = second;
    }
    CompressedPair(F&& first, const S& second) {
        first_ = std::move(first);
        second_ = second;
    }
    CompressedPair(const F& first, S&& second) {
        first_ = first;
        second_ = std::move(second);
    }
    CompressedPair(F&& first, S&& second) {
        first_ = std::move(first);
        second_ = std::move(second);
    }

    F& GetFirst() {
        return first_;
    }
    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false, true> : F {
public:
    CompressedPair() : second_() {
    }
    CompressedPair(const F& first, const S& second) : F(first) {
        second_ = second;
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)) {
        second_ = second;
    }
    CompressedPair(const F& first, S&& second) : F(first) {
        second_ = std::move(second);
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)) {
        second_ = std::move(second);
    }

    F& GetFirst() {
        return *this;
    }
    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return *this;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false, false> : F {
public:
    CompressedPair() : second_() {
    }
    CompressedPair(const F& first, const S& second) : F(first) {
        second_ = second;
    }
    CompressedPair(F&& first, const S& second) : F(std::move(first)) {
        second_ = second;
    }
    CompressedPair(const F& first, S&& second) : F(first) {
        second_ = std::move(second);
    }
    CompressedPair(F&& first, S&& second) : F(std::move(first)) {
        second_ = std::move(second);
    }

    F& GetFirst() {
        return *this;
    }
    S& GetSecond() {
        return second_;
    }

    const F& GetFirst() const {
        return *this;
    };

    const S& GetSecond() const {
        return second_;
    };

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true, true> : S {
public:
    CompressedPair() : first_() {
    }
    CompressedPair(const F& first, const S& second) : S(second) {
        first_ = first;
    }
    CompressedPair(F&& first, const S& second) : S(second) {
        first_ = std::move(first);
    }
    CompressedPair(const F& first, S&& second) : S(std::move(second)) {
        first_ = first;
    }
    CompressedPair(F&& first, S&& second) : S(std::move(second)) {
        first_ = std::move(first);
    }

    F& GetFirst() {
        return first_;
    }
    S& GetSecond() {
        return *this;
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return *this;
    };

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true, false> : S {
public:
    CompressedPair() : first_() {
    }
    CompressedPair(const F& first, const S& second) : S(second) {
        first_ = first;
    }
    CompressedPair(F&& first, const S& second) : S(second) {
        first_ = std::move(first);
    }
    CompressedPair(const F& first, S&& second) : S(std::move(second)) {
        first_ = first;
    }
    CompressedPair(F&& first, S&& second) : S(std::move(second)) {
        first_ = std::move(first);
    }
    F& GetFirst() {
        return first_;
    }
    S& GetSecond() {
        return *this;
    }

    const F& GetFirst() const {
        return first_;
    };

    const S& GetSecond() const {
        return *this;
    };

private:
    F first_;
};
