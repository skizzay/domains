#pragma once

#include <cstdint>
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

template <std::size_t, class>
struct get;

template <std::size_t I, class Head, class... Tail>
struct get<I, parameter_pack<Head, Tail...>> : get<I - 1, parameter_pack<Tail...>> {};

template <class Head, class... Tail>
struct get<0, parameter_pack<Head, Tail...>> {
   using type = Head;
};

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
}
