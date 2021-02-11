#pragma once

#include <chrono>

namespace domains {
template <bool B>
struct basic_null_mutex {
   void const *native_handle() const noexcept {
      return nullptr;
   }

   void lock() noexcept {
   }

   void lock_shared() noexcept {
   }

   bool try_lock() noexcept {
      return B;
   }

   bool try_lock_shared() noexcept {
      return B;
   }

   template <class Rep, class Period>
   bool try_lock_for(std::chrono::duration<Rep, Period> const &) noexcept {
      return B;
   }

   template <class Rep, class Period>
   bool try_lock_shared_for(std::chrono::duration<Rep, Period> const &) noexcept {
      return B;
   }

   template <class Clock, class Duration>
   bool try_lock_until(std::chrono::time_point<Clock, Duration> const &) noexcept {
      return B;
   }

   template <class Clock, class Duration>
   bool try_lock_shared_until(std::chrono::time_point<Clock, Duration> const &) noexcept {
      return B;
   }

   void unlock() noexcept {
   }

   void unlock_shared() noexcept {
   }
};

using null_mutex = basic_null_mutex<true>;
using failed_null_mutex = basic_null_mutex<false>;
}
