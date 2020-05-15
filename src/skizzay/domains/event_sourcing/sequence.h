#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<class Tag, class T>
class basic_sequence {
    static_assert(std::is_unsigned_v<T> && std::is_integral_v<T> && !std::is_same_v<T, bool>,
        "T must be an unsigned integer");

    T value_;

public:
    using type = T;

    constexpr basic_sequence(T const t = {}) noexcept :
        value_{t}
    {
    }

    constexpr basic_sequence(basic_sequence<Tag, T> const &) noexcept = default;
    constexpr basic_sequence<Tag, T> &operator =(basic_sequence<Tag, T> const &) noexcept = default;

    static constexpr basic_sequence<Tag, T> max() noexcept {
        return {std::numeric_limits<T>::max()};
    }

    constexpr T value() const noexcept {
        return value_;
    }

    constexpr basic_sequence<Tag, T> next() const noexcept {
        return {value_ + 1};
    }

    constexpr bool operator==(basic_sequence<Tag, T> const r) const noexcept {
        return value() == r.value();
    }

    constexpr bool operator!=(basic_sequence<Tag, T> const r) const noexcept {
        return value() != r.value();
    }

    constexpr bool operator< (basic_sequence<Tag, T> const r) const noexcept {
        return value() < r.value();
    }

    constexpr bool operator> (basic_sequence<Tag, T> const r) const noexcept {
        return value() > r.value();
    }

    constexpr bool operator<=(basic_sequence<Tag, T> const r) const noexcept {
        return value() <= r.value();
    }

    constexpr bool operator>=(basic_sequence<Tag, T> const r) const noexcept {
        return value() >= r.value();
    }
};

}

namespace std {

template<class Tag, class T>
struct hash<skizzay::domains::event_sourcing::basic_sequence<Tag, T>> {
   using argument_type = skizzay::domains::event_sourcing::basic_sequence<Tag, T>;
   using result_type = size_t;

   constexpr size_t operator()(argument_type const arg) const noexcept {
       return hash<T>{}(arg.value());
   }
};

}