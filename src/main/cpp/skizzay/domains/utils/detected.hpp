#pragma once

#include <type_traits>

namespace domains {
namespace details_ {
template<template<class ...> class, class, class ...>
struct is_detected_impl : std::false_type {};

template<template<class ...> class Trait, class ...Args>
struct is_detected_impl<Trait, std::void_t<Trait<Args...>>, Args...> : std::true_type {};
}

template<template<class ...> class Trait, class ...Args>
using is_detected = details_::is_detected_impl<Trait, void, Args...>;

template<template<class ...> class Trait, class ...Args>
constexpr bool is_detected_v = is_detected<Trait, Args...>::value;
}
