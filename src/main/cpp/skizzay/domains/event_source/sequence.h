#pragma once

#include <concepts>

namespace skizzay::domains::event_source {

template<class Tag, std::integral T>
class sequence {
    T value_;

public:
    using value_type = T;

    constexpr sequence() noexcept :
        sequence{static_cast<T>(0)}
    {
    }

    constexpr explicit sequence(T const value) noexcept :
        value_{value}
    {
    }

    constexpr T value() const noexcept {
        return value_;
    }

    constexpr auto operator<=>(sequence const &) const noexcept = default;

    constexpr sequence<Tag, T> next() const noexcept {
        return sequence<Tag, T>{value_ + static_cast<T>(1)};
    }

    constexpr sequence<Tag, T> previous() const noexcept {
        return sequence<Tag, T>{value_ - static_cast<T>(1)};
    }
};

}