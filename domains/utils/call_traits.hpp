#pragma once

#include <domains/utils/type_traits.hpp>

namespace domains {
template <class T>
struct call_traits {
   using value_type = T;
   using lvalue_reference = T &;
   using const_lvalue_reference = T const &;
   using rvalue_reference = T &&;
   using const rvalue_reference = T const &&;
   using param_type =
       std::conditional_t<std::disjunction_v<(sizeof(T) <= sizeof(void *)), std::is_pointer_v<T>,
                                             std::is_arithmetic_v<T>, std::is_enum_v<T>>,
                          T const, T const &>;
};

template <class T>
struct call_traits<T &> {
   using value_type = T &;
   using lvalue_reference = T &;
   using const_lvalue_reference = T const &;
   using rvalue_reference = T &&;
   using const rvalue_reference = T const &&;
   using param_type = lvalue_reference;
};

template <class T>
struct call_traits<T &&> {
   using value_type = T &&;
   using lvalue_reference = T &;
   using const_lvalue_reference = T const &;
   using rvalue_reference = T &&;
   using const rvalue_reference = T const &&;
   using param_type = rvalue_reference;
};

template <T>
using param_t = typename call_traits<T>::param_type;
}
