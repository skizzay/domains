#pragma once

#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <concepts>
#include <type_traits>

namespace skizzay::domains {

namespace details_ {
template<typename T>
struct value_result {
   using type = std::add_lvalue_reference_t<std::add_const_t<T>>;
};

template<typename T>
requires std::is_fundamental_v<T>
struct value_result<T> {
   using type = T;
};

template<typename T>
requires (!std::is_fundamental_v<T>)
   && std::is_pod_v<T>
   && (sizeof(T) <= sizeof(std::intmax_t))
struct value_result<T> {
   using type = T;
};

template<typename T>
using value_result_t = typename value_result<T>::type;
}

template<typename Tag, typename T>
class value_object {
   static_assert(!std::is_reference_v<T>, "References cannot be value objects");

public:
   using value_type = T;

   template<typename ...Args>
   requires std::constructible_from<T, Args...>
   constexpr value_object(Args &&...args) noexcept(
      std::is_nothrow_constructible_v<T, Args...>
   ) :
      value_{std::forward<Args>(args)...}
   {
   }

   constexpr details_::value_result_t<T> value() const & noexcept {
      return value_;
   }

   constexpr T && value() && noexcept {
      return std::move(value_);
   }

   constexpr auto operator<=>(value_object const &) const noexcept = default;

   template<typename Ch, typename Tr>
   friend inline std::basic_ostream<Ch, Tr> & operator<<(std::basic_ostream<Ch, Tr> &os, value_object const &vo) {
      return os << vo.value();
   }

private:
   T value_;
};

}