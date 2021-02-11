#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace domains {

template <class... Args>
struct parameter_pack {
   static constexpr std::size_t size = sizeof...(Args);
};

template <class, class>
struct append;

template <class T, class... List>
struct append<T, parameter_pack<List...>> {
   using type = parameter_pack<List..., T>;
};

template <class>
struct reverse;

template <class T, class... List>
struct reverse<parameter_pack<T, List...>> {
   using type = typename append<T, typename reverse<parameter_pack<List...>>::type>::type;
};

template <class ParameterPack>
using reverse_t = typename reverse<ParameterPack>::type;

template <std::size_t, class>
struct get;

template <std::size_t I, class Head, class... Tail>
struct get<I, parameter_pack<Head, Tail...>> : get<I - 1, parameter_pack<Tail...>> {};

template <class Head, class... Tail>
struct get<0, parameter_pack<Head, Tail...>> {
   using type = Head;
};

template <std::size_t I, class ParameterPack>
using get_t = typename get<I, ParameterPack>::type;

#if 0
template <class, class>
struct get;

template <class T, class Head, class... Tail>
struct get<T, parameter_pack<Head, Tail...>> : get<T, parameter_pack<Tail...>> {};

template <class Head, class... Tail>
struct get<Head, parameter_pack<Head, Tail...>> {
   using type = Head;
};
#endif

template <class, class>
struct in;

template <class T>
struct in<T, parameter_pack<>> : std::false_type {};

template <class T, class... Tail>
struct in<T, parameter_pack<T, Tail...>> : std::true_type {};

template <class T, class Head, class... Tail>
struct in<T, parameter_pack<Head, Tail...>> : in<T, parameter_pack<Tail...>> {};

template <class T, class ParameterPack>
constexpr bool in_v = in<T, ParameterPack>::value;

namespace details_ {
template <class, class>
struct is_unique_impl;

template <class T>
struct is_unique_impl<T, parameter_pack<>> : std::true_type {};

template <class T, class U, class... Us>
struct is_unique_impl<T, parameter_pack<U, Us...>>
    : std::integral_constant<bool, !in<T, parameter_pack<U, Us...>>::value &&
                                       is_unique_impl<U, parameter_pack<Us...>>::value> {};
}

template <class>
struct is_unique;

template <class Head, class... Tail>
struct is_unique<parameter_pack<Head, Tail...>>
    : details_::is_unique_impl<Head, parameter_pack<Tail...>> {};

template <class... List>
constexpr bool is_unique_v = is_unique<parameter_pack<List...>>::value;

template <class>
struct as_tuple;

template <class... Args>
struct as_tuple<parameter_pack<Args...>> {
   using type = std::tuple<Args...>;
};

template <class... Args>
using as_tuple_t = typename as_tuple<parameter_pack<Args...>>::type;

template <class>
struct as_parameter_pack;

template <class... Args>
struct as_parameter_pack<std::tuple<Args...>> {
   using type = parameter_pack<Args...>;
};

template <class... Args>
using as_parameter_pack_t = typename as_parameter_pack<std::tuple<Args...>>::type;
}
