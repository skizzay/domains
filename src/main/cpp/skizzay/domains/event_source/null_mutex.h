#pragma once

#include <cstddef>

namespace skizzay::domains::event_source {

template<bool try_result=true>
struct null_mutex final {
   using native_handle_type = std::nullptr_t;

   constexpr native_handle_type native_handle() const noexcept {
      return nullptr;
   }

   constexpr void lock() const noexcept {
   }

   constexpr void unlock() const noexcept {
   }

   constexpr bool try_lock() const noexcept {
      return try_result;
   }

   template<typename Rep, typename Period>
   constexpr bool try_lock_for([[maybe_unused]] std::chrono::duration<Rep, Period> const &timeout_duration) const noexcept {
      return try_result;
   }

   template< class Clock, class Duration >
   constexpr bool try_lock_until([[maybe_unused]] std::chrono::time_point<Clock, Duration> const &timeout_time) const noexcept {
      return try_result;
   }

   constexpr void lock_shared() const noexcept {
   }

   constexpr void unlock_shared() const noexcept {
   }

   constexpr bool try_lock_shared() const noexcept {
      return try_result;
   }

   template<typename Rep, typename Period>
   constexpr bool try_lock_shared_for([[maybe_unused]] std::chrono::duration<Rep, Period> const &timeout_duration) const noexcept {
      return try_result;
   }

   template< class Clock, class Duration >
   constexpr bool try_lock_shared_until([[maybe_unused]] std::chrono::time_point<Clock, Duration> const &timeout_time) const noexcept {
      return try_result;
   }
};

}