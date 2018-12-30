#pragma once

#include <skizzay/utilz/traits.h>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace skizzay::domains::event_sourcing {

template<class T>
class range_holder {
   T range_;
   std::size_t size_;

   utilz::strip_reference_wrapper_t<T> &reference() noexcept {
      return utilz::strip_reference_wrapper<T>::apply(range_);
   }

   utilz::strip_reference_wrapper_t<std::add_const_t<T>> &const_reference() const noexcept {
      return utilz::strip_reference_wrapper<std::add_const_t<T>>::apply(range_);
   }

public:
   constexpr range_holder(T &&r, std::size_t const n)
      noexcept(std::is_nothrow_move_constructible_v<T>) :
      range_{std::move(r)},
      size_{n}
   {
   }

   constexpr range_holder(T const &r, std::size_t const n)
      noexcept(std::is_nothrow_copy_constructible_v<T>) :
      range_{r},
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
      return std::begin(reference());
   }

   constexpr auto begin() const {
      return std::begin(const_reference());
   }

   constexpr auto cbegin() const {
      return std::begin(const_reference());
   }

   constexpr auto end() {
      return std::end(reference());
   }

   constexpr auto end() const {
      return std::end(const_reference());
   }

   constexpr auto cend() const {
      return std::end(const_reference());
   }
};

}
