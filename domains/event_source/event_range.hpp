#pragma once

#include <domains/aggregate/event.hpp>

namespace domains {
template <typename T>
concept bool EventRange = InputRange<T> && requires(T const t) {
   requires Event<std::decay_t<decltype(std::begin(t))>>;
};
}
