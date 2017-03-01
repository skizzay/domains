#pragma once

#include <domains/messaging/buffer.hpp>

namespace domains {
template<class ...> struct dispatcher;

template <class T>
struct dispatcher<T> : T {
   using T::operator();

   explicit dispatcher(T &&t) :
      T(std::forward<T>(t))
   {
   }
};

template <class T, class ...Ts>
struct dispatcher<T, Ts...> : T, dispatcher<Ts...> {
   using T::operator();
   using dispatcher<Ts...>::operator();

   explicit dispatcher(T &&t, Ts &&...ts) :
      T(std::forward<T>(t)),
      dispatcher<Ts...>(std::forward<Ts>(ts)...)
   {
   }
};

template <class ...Ts>
dispatcher<Ts...> make_dispatcher(Ts &&...ts) {
   return dispatcher<Ts...>(std::forward<Ts>(ts)...);
}
}
