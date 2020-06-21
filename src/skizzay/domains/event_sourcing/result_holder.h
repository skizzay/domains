#pragma once

#include <compare>
#include <cstddef>
#include <functional>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<class T>
class result_holder
      requires (std::is_enum_v<T>)
{
   T t_;

public:
   result_holder() = delete;

   constexpr result_holder(T const t) noexcept :
      t_{t}
   {
   }

   constexpr result_holder(result_holder const &other) noexcept = default;
   constexpr result_holder &operator =(result_holder const &other) noexcept = default;

   constexpr char const * to_string() const noexcept {
      return to_string(t_);
   }

   constexpr std::size_t hash_code() const noexcept {
      return std::hash<std::underlying_type_t<T>>{t_}();
   }

   constexpr T result_value() const noexcept {
      return t_;
   }

   constexpr auto operator<=>(result_holder<T> const &) const noexcept = default;

   constexpr auto operator<=>(T const t) const noexcept {
      return result_value() <=> t;
   }
};

template<class T>
constexpr auto operator<=>(T const l, result_holder<T> const r) noexcept {
   return l <=> r.result_value();
}

}

namespace std {

template<class T>
struct hash<skizzay::domains::event_sourcing::result_holder<T>> {
   using argument_type = skizzay::domains::event_sourcing::result_holder<T>;
   using result_type = std::size_t;

   constexpr std::size_t operator()(argument_type const arg) const noexcept {
      return arg.hash_code();
   }
};

}
