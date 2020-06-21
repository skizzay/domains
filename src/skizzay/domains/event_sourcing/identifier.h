#pragma once

#include "skizzay/domains/event_sourcing/concepts/tag.h"
#include <skizzay/utilz/traits.h>
#include <compare>

namespace skizzay::domains::event_sourcing {
template<concepts::tag Tag, std::totally_ordered T>
class basic_identifier {
    T value_;

public:
    using tag_type = Tag;
    using value_type = T;

    constexpr basic_identifier(T t={}) noexcept(std::is_nothrow_move_constructible_v<T>)
    : value_{std::move(t)}
    {
    }

    constexpr auto operator<=>(basic_identifier<Tag, T> const &rhs) const = default;

    constexpr value_type const & value() const {
        return value_;
    }
};

template<concepts::tag Tag, std::totally_ordered T>
inline constexpr basic_identifier<Tag, T> unidentified = {};

namespace concepts {
    template<class T>
    concept identifier = utilz::is_template_v<T, basic_identifier>;
}
}