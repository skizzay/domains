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

namespace details_ {
template<typename T>
using with_reference = T&;
}

template<typename T>
concept referenceable = requires { typename details_::with_reference<T>; };

template<typename T>
concept dereferenceable = requires (T &t) {
    { *t } -> referenceable;
};

template<typename T, typename U>
concept same_reference_removed = std::same_as<std::remove_reference_t<T>, std::remove_reference_t<U>>;

}

namespace skizzay::domains {

template <concepts::dereferenceable D>
using dereferenced_t = decltype(*std::declval<D &>());

template<typename T>
constexpr decltype(auto) get_reference(T &t) noexcept {
    if constexpr (concepts::dereferenceable<T>) {
        return *t;
    }
    else {
        return t;
    }
}


template<typename>
struct is_reference_wrapper : std::false_type {
};


template<typename T>
struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {
};


template<typename T>
inline constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

}