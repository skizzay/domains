#pragma once

#include "skizzay/domains/event_sourcing/stream_version.h"
#include <skizzay/utilz/traits.h>
#include <chrono>
#include <cstdint>

namespace skizzay::domains::event_sourcing {

template<class StreamIdType, class StreamVersionIntegral, class TimestampType>
class event_header {
   static_assert(utilz::equivalent_v<StreamIdType>,
         "StremIdType must be equivalent");
   static_assert(utilz::is_time_point_v<TimestampType>,
         "TimestampType must be a std::chrono::time_point");

   StreamIdType stream_id_;
   basic_stream_version<StreamVersionIntegral> stream_version_;
   TimestampType timestamp_;

public:
   using stream_id_type = StreamIdType;
   using stream_version_type = basic_stream_version<StreamVersionIntegral>;
   using timestamp_type = TimestampType;

   constexpr event_header(
         stream_id_type si,
         stream_version_type const sv,
         timestamp_type const ts) noexcept :
      stream_id_{std::move(si)},
      stream_version_{sv},
      timestamp_{ts}
   {
   }

   constexpr stream_id_type const &stream_id() const noexcept {
      return stream_id_;
   }

   constexpr stream_version_type stream_version() const noexcept {
      return stream_version_;
   }

   constexpr timestamp_type timestamp() const noexcept {
      return timestamp_;
   }
};

template<class StreamIdType, class StreamVersionIntegral, class TimestampType>
event_header(StreamIdType, basic_stream_version<StreamVersionIntegral>, TimestampType) -> event_header<StreamIdType, StreamVersionIntegral, TimestampType>;

}
