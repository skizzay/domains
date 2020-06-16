#pragma once

#include "skizzay/domains/event_sourcing/concepts/tag.h"

#include <skizzay/utilz/traits.h>
#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<concepts::tag Tag, std::unsigned_integral T>
class basic_sequence {
    T value_;

public:
    using tag_type = Tag;
    using value_type = T;

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
    
    constexpr auto operator<=>(basic_sequence<Tag, T> const rhs) const {
        return this->value() <=> rhs.value();
    }

    constexpr bool operator==(basic_sequence<Tag, T> const rhs) const {
        return this->value() == rhs.value();
    }

    constexpr bool operator!=(basic_sequence<Tag, T> const rhs) const {
        return this->value() != rhs.value();
    }
};


template<std::unsigned_integral T>
using basic_stream_version = basic_sequence<utilz::tag<struct stream_version_tag>, T>;


template<std::unsigned_integral T>
using basic_commit_sequence = basic_sequence<utilz::tag<struct commit_sequence_tag>, T>;

template<concepts::tag Tag, std::unsigned_integral T>
inline constexpr basic_sequence<Tag, T> unsequenced = basic_sequence<Tag, T>{};

namespace concepts {
    template<class T>
    concept sequence = utilz::is_template_v<T, basic_sequence>;

    template<class T>
    concept stream_version = utilz::is_template_v<T, basic_sequence> && std::is_same_v<skizzay::utilz::tag<struct skizzay::domains::event_sourcing::stream_version_tag>, typename T::tag_type>;

    template<class T>
    concept commit_sequence = utilz::is_template_v<T, basic_sequence> && std::is_same_v<skizzay::utilz::tag<struct skizzay::domains::event_sourcing::commit_sequence_tag>, typename T::tag_type>;
}

}

namespace std {

template<skizzay::domains::event_sourcing::concepts::tag Tag, unsigned_integral T>
struct hash<skizzay::domains::event_sourcing::basic_sequence<Tag, T>> {
   using argument_type = skizzay::domains::event_sourcing::basic_sequence<Tag, T>;
   using result_type = size_t;

   constexpr result_type operator()(argument_type const arg) const noexcept {
       return hash<T>{}(arg.value());
   }
};

}