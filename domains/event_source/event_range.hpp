#pragma once

#include <domains/aggregate/event.hpp>

namespace domains {
template <typename T>
concept bool EventRange = InputRange<T> && requires(T t) {
   { *t.begin() } const noexcept -> Event;
};
}
