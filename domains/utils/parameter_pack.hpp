#pragma once

#include <cstdint>

namespace domains {

template<class... Args>
struct parameter_pack {
   static constexpr std::size_t size = sizeof...(Args);
};


template<class, class> struct append;

template<class T, class ... List>
struct append<T, parameter_pack<List...>> {
   using type = parameter_pack<List..., T>;
};


template<class ...> struct reverse;

template<>
struct reverse_parameter_pack<> {
   using type = parameter_pack<>;
};

template<class T, class ... List>
struct reverse<T, List...> {
   using type = typename append<
         T,
         typename reverse<List...>::type
      >::type;
};


template<std::size_t, class> struct get;

template<std::size_t I, class Head, class ... Tail>
struct get<I, parameter_pack<Head, Tail...>> : get<I - 1, parameter_pack<Tail...>>{
};

template<class Head, class ... Tail>
struct get<0, parameter_pack<Head, Tail...>> {
   using type = Head;
};


template<class, class> struct get;

template<class T, class Head, class ... Tail>
struct get<T, parameter_pack<Head, Tail...>> : get<T, parameter_pack<Tail...>>{
};

template<class Head, class ... Tail>
struct get<Head, parameter_pack<Head, Tail...>> {
   using type = Head;
};

}
