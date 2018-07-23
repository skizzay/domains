#pragma once

#include <chrono>
#include <domains/utils/concepts.hpp>

namespace domains {

template <typename T>
concept bool Event = SemiRegular<T> && requires(T const t) {
   { t.timestamp() } noexcept -> TimePoint;
   { t.sequence_number() } noexcept -> UnsignedIntegral;
   { t.stream_id() } noexcept -> UnsignedIntegral;
};

template <typename T>
struct event_traits {
   using stream_id_type = std::uint64_t;
   using stream_version_type = std::uint32_t;
   using timestamp_type =
       std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>;
};

// Mixin class to make your class an Event
template <typename T>
class event {
   typename event_traits<T>::stream_id_type stream_id_;
   typename event_traits<T>::timestamp_type timestamp_;
   typename event_traits<T>::stream_version_type stream_version_;

protected:
   constexpr event(typename event_traits<T>::stream_id_type const si,
         typename event_traits<T>::timestamp_type const t,
         typename event_traits<T>::stream_version_type const sv) noexcept :
      stream_id_(si),
      timestamp_(t),
      stream_version_(sv)
   {
   }

public:
   constexpr auto stream_id() const noexcept {
      return stream_id_;
   }

   constexpr auto stream_version() const noexcept {
      return stream_version_;
   }

   constexpr auto timestamp() const noexcept {
      return timestamp_;
   }
};

}
