#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<class T>
class result_holder {
   static_assert(std::is_enum_v<T>, "T must be an enum");

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

   constexpr bool operator==(result_holder<T> const other) const noexcept {
      return t_ == other.t_;
   }

   constexpr bool operator!=(result_holder<T> const other) const noexcept {
      return t_ != other.t_;
   }

   constexpr bool operator<=(result_holder<T> const other) const noexcept {
      return t_ <= other.t_;
   }

   constexpr bool operator>=(result_holder<T> const other) const noexcept {
      return t_ >= other.t_;
   }

   constexpr bool operator< (result_holder<T> const other) const noexcept {
      return t_ < other.t_;
   }

   constexpr bool operator> (result_holder<T> const other) const noexcept {
      return t_ > other.t_;
   }

   constexpr bool operator==(T const other) const noexcept {
      return t_ == other;
   }

   constexpr bool operator!=(T const other) const noexcept {
      return t_ != other;
   }

   constexpr bool operator<=(T const other) const noexcept {
      return t_ <= other;
   }

   constexpr bool operator>=(T const other) const noexcept {
      return t_ >= other;
   }

   constexpr bool operator< (T const other) const noexcept {
      return t_ < other;
   }

   constexpr bool operator> (T const other) const noexcept {
      return t_ > other;
   }
};

template<class T>
constexpr bool operator==(T const l, result_holder<T> const r) noexcept {
   return l == r.result_value();
}

template<class T>
constexpr bool operator!=(T const l, result_holder<T> const r) noexcept {
   return l != r.result_value();
}

template<class T>
constexpr bool operator<=(T const l, result_holder<T> const r) noexcept {
   return l <= r.result_value();
}

template<class T>
constexpr bool operator>=(T const l, result_holder<T> const r) noexcept {
   return l >= r.result_value();
}

template<class T>
constexpr bool operator< (T const l, result_holder<T> const r) noexcept {
   return l < r.result_value();
}

template<class T>
constexpr bool operator> (T const l, result_holder<T> const r) noexcept {
   return l > r.result_value();
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
