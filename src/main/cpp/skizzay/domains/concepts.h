#pragma once

#include <concepts>
#include <chrono>

namespace skizzay::domains::concepts {

template<class T>
concept identifier = std::totally_ordered<T> && std::copyable<T>;

namespace details_ {
template<class T>
struct is_timestamp_helper : std::false_type {
};

template<class Clock, class Duration>
struct is_timestamp_helper<std::chrono::time_point<Clock, Duration>> : std::true_type {
};
}

template<class T>
concept timestamp = details_::is_timestamp_helper<T>::value;

template<class T>
concept value_object = std::regular<T> && requires(T const &t) {
    t.value();
};

template<class T>
concept sequenced = value_object<T> && std::totally_ordered<T> && requires(T const &t) {
    typename T::value_type;
    { t.value() } -> std::integral;
    { t.next() } -> std::same_as<T>;
    { t.previous() } -> std::same_as<T>;
};

}