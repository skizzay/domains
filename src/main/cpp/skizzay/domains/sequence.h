#pragma once

#include <skizzay/domains/value_object.h>
#include <concepts>

namespace skizzay::domains {

template<class Tag, std::integral T>
class sequence : public value_object<Tag, T> {
public:
    using value_object<Tag, T>::value_object;

    constexpr sequence<Tag, T> next() const noexcept {
        return sequence<Tag, T>{this->value() + static_cast<T>(1)};
    }

    constexpr sequence<Tag, T> previous() const noexcept {
        return sequence<Tag, T>{this->value() - static_cast<T>(1)};
    }
};

}