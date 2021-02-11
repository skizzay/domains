#pragma once

// clang-format off
#if __has_include(<experimental/type_traits>)
// clang-format on
#include <experimental/type_traits>

namespace domains {
inline namespace v1 {
using std::experimental::is_signed_v;
using std::experimental::is_unsigned_v;
using std::experimental::is_integral_v;
using std::experimental::is_nothrow_constructible_v;
using std::experimental::is_nothrow_move_constructible_v;
using std::experimental::is_same_v;
using std::experimental::is_base_of_v;
}
}

#else
#include <type_traits>

namespace domains {
inline namespace v2 {
using std::is_signed_v;
using std::is_unsigned_v;
using std::is_integral_v;
using std::is_nothrow_constructible_v;
using std::is_nothrow_move_constructible_v;
using std::is_same_v;
using std::is_base_of_v;
}
}

#endif
#include <chrono>
#include <utility>
#include "domains/utils/detected.hpp"

namespace domains {
template <class T, class U, template <class> class TQual, template <class> class UQual>
struct basic_common_reference {};

namespace details_ {
inline namespace v1 {
template <class T>
using t_ = typename T::type;

template <class T>
constexpr typename t_<T>::value_type v_ = t_<T>::value;

template <class T, class... Args>
using apply_ = typename T::template apply<Args...>;

template <class T, class U>
struct compose_ {
   template <class V>
   using apply = apply_<T, apply_<U, V>>;
};

template <class T>
struct id_ {
   using type = T;
};

template <template <class...> class T, class... U>
concept bool _Valid = requires {
   typename T<U...>;
};

template <class U, template <class...> class T, class... V>
concept bool _Is = _Valid<T, U, V...> &&v_<T<U, V...>>;

template <class U, class V>
concept bool _ConvertibleTo = _Is<U, std::is_convertible, V>;

template <template <class...> class T, class... U>
struct defer_ {};
_Valid {
   T, ... U
}
struct defer_<T, U...> : id_<T<U...>> {};

template <template <class...> class T>
struct q_ {
   template <class... U>
   using apply = t_<defer_<T, U...>>;
};

template <class T>
struct has_type_ : std::false_type {};
template <class T>
requires _Valid<t_, T> struct has_type_<T> : std::true_type {};

template <class T, class X = std::remove_reference_t<T>>
using cref_ = std::add_lvalue_reference_t<std::add_const_t<X>>;
template <class T>
using uncvref_ = std::remove_cv_t<std::remove_reference_t<T>>;

template <class T, class U>
using cond_ = decltype(true ? std::declval<T>() : std::declval<U>());

template <class From, class To>
struct copy_cv__ : id_<To> {};
template <class From, class To>
struct copy_cv__<From const, To> : std::add_const<To> {};
template <class From, class To>
struct copy_cv__<From volatile, To> : std::add_volatile<To> {};
template <class From, class To>
struct copy_cv__<From const volatile, To> : std::add_cv<To> {};
template <class From, class To>
using copy_cv_ = t_<copy_cv__<From, To>>;

template <class T, class U>
struct builtin_common_ {};
template <class T, class U>
using builtin_common_t_ = t_<builtin_common_<T, U>>;
template <class T, class U>
requires _Valid<cond_, cref_<T>, cref_<U>> struct builtin_common_<T, U>
    : std::decay<cond_<cref_<T>, cref_<U>>> {};
template <class T, class U, class R = builtin_common_t_<T &, U &>>
using rref_res_ = std::conditional_t<v_<std::is_reference<R>>, std::remove_reference_t<R> &&, R>;
template <class T, class U>
requires _Valid<builtin_common_t_, T &, U &> &&_ConvertibleTo<T &&, rref_res_<T, U>>
    &&_ConvertibleTo<U &&, rref_res_<T, U>> struct builtin_common_<T &&, U &&>
    : id_<rref_res_<T, U>> {};
template <class T, class U>
using lref_res_ = cond_<copy_cv_<T, U> &, copy_cv_<U, T> &>;
template <class T, class U>
struct builtin_common_<T &, U &> : defer_<lref_res_, T, U> {};
template <class T, class U>
requires _Valid<builtin_common_t_, T &, U const &>
    &&_ConvertibleTo<U &&, builtin_common_t_<T &, U const &>> struct builtin_common_<T &, U &&>
    : builtin_common_<T &, U const &> {};
template <class T, class U>
struct builtin_common_<T &&, U &> : builtin_common_<U &, T &&> {};

// common_type
template <class... Ts>
struct common_type {};

template <class... T>
using common_type_t = t_<common_type<T...>>;

template <class T>
struct common_type<T> : std::decay<T> {};

template <class T>
concept bool _Decayed = v_<std::is_same<std::decay_t<T>, T>>;

template <class T, class U>
struct common_type2_ : common_type<std::decay_t<T>, std::decay_t<U>> {};

template <_Decayed T, _Decayed U>
struct common_type2_<T, U> : builtin_common_<T, U> {};

template <class T, class U>
struct common_type<T, U> : common_type2_<T, U> {};

template <class T, class U, class V, class... W>
requires _Valid<common_type_t, T, U> struct common_type<T, U, V, W...>
    : common_type<common_type_t<T, U>, V, W...> {};

namespace qual_ {
using rref_ = q_<std::add_rvalue_reference_t>;
using lref_ = q_<std::add_lvalue_reference_t>;
template <class>
struct xref_ : id_<compose_<q_<t_>, q_<id_>>> {};
template <class T>
struct xref_<T &> : id_<compose_<lref_, t_<xref_<T>>>> {};
template <class T>
struct xref_<T &&> : id_<compose_<rref_, t_<xref_<T>>>> {};
template <class T>
struct xref_<const T> : id_<q_<std::add_const_t>> {};
template <class T>
struct xref_<volatile T> : id_<q_<std::add_volatile_t>> {};
template <class T>
struct xref_<const volatile T> : id_<q_<std::add_cv_t>> {};
}

template <class T, class U>
using basic_common_reference_ =
    basic_common_reference<uncvref_<T>, uncvref_<U>, qual_::xref_<T>::type::template apply,
                           qual_::xref_<U>::type::template apply>;


// common_reference
template <class... T>
struct common_reference {};

template <class... T>
using common_reference_t = t_<common_reference<T...>>;

template <class T>
struct common_reference<T> : id_<T> {};

template <class T, class U>
struct common_reference2_ : std::conditional_t<v_<has_type_<basic_common_reference_<T, U>>>,
                                                basic_common_reference_<T, U>, common_type<T, U>> {
};

template <class T, class U>
requires _Valid<builtin_common_t_, T, U>
    &&_Is<builtin_common_t_<T, U>, std::is_reference> struct common_reference2_<T, U>
    : builtin_common_<T, U> {};

template <class T, class U>
struct common_reference<T, U> : common_reference2_<T, U> {};

template <class T, class U, class V, class... W>
requires _Valid<common_reference_t, T, U> struct common_reference<T, U, V, W...>
    : common_reference<common_reference_t<T, U>, V, W...> {};
}
}

template <class... Ts>
struct common_reference : details_::common_reference<Ts...> {};

template <class... Ts>
using common_reference_t = details_::common_reference_t<Ts...>;

template <template <class...> class F, class... T>
concept bool as_concept = F<T...>::value;

template <typename T>
using value_type_t = typename T::value_type;

template <typename T>
using reference_t = typename T::reference;

template <typename T>
using rvalue_reference_t = typename T::rvalue_reference;

template <typename T>
using difference_type_t = typename T::difference_type;

template <typename T>
using size_type_t = typename T::size_type;

template <typename T>
using iterator_category_t = typename T::iterator_category;

template <typename T>
using iterator_t = typename T::iterator;

template <typename T>
using const_iterator_t = typename T::const_iterator;

template <typename T>
using sentinel_t = decltype(std::end(std::declval<T>()));

namespace traits {
inline namespace v1 {
template <typename> struct is_time_point : std::false_type {};

template <typename Clock, typename Duration>
struct is_time_point<std::chrono::time_point<Clock, Duration>> : std::true_type {};

namespace details_ {
template<template<class ...> class Trait, class ...Args>
using unsigned_result = std::conjunction<
      is_detected<Trait, Args...>,
      std::is_unsigned<Trait<Args...>>
   >;

template<class T>
using version_member_function = decltype(std::declval<T const>().version());

template<class T>
using unsigned_version = unsigned_result<version_member_function, T>;

template<class T>
using timestamp_member_function = decltype(std::declval<T const>().timestamp());

template<class T>
using timestamp = std::conjunction<
      is_detected<timestamp_member_function, T>,
      is_time_point<timestamp_member_function<T>>
   >;

template<class T>
using sequence_number_member_function = decltype(std::declval<T const >().sequence_number());

template<class T>
using unsigned_sequence_number = unsigned_result<sequence_number_member_function, T>;
}

template<class T>
using id = decltype(std::declval<T const>().id());

template<typename T>
using entity = std::conjunction<
      details_::unsigned_version<T>,
      is_detected<id, T>
   >;
}
}
}
