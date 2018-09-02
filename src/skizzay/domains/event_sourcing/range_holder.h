#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace skizzay::domains::event_sourcing {

template<class T>
class range_holder {
   T range_;
   std::size_t size_;

public:
   constexpr range_holder(T r, std::size_t const n)
      noexcept(std::is_nothrow_move_constructible_v<T>) :
      range_{std::move(r)},
      size_{n}
   {
   }

   constexpr std::size_t size() const noexcept {
      return size_;
   }

   constexpr bool empty() const noexcept {
      return 0 == size();
   }

   constexpr auto begin() {
      return std::begin(range_);
   }

   constexpr auto begin() const {
      return std::begin(range_);
   }

   constexpr auto cbegin() const {
      return std::begin(range_);
   }

   constexpr auto end() {
      return std::end(range_);
   }

   constexpr auto end() const {
      return std::end(range_);
   }

   constexpr auto cend() const {
      return std::end(range_);
   }
};

}
