#pragma once

#include "skizzay/domains/event_sourcing/stream_version.h"
#include "skizzay/domains/event_sourcing/traits.h"
#include "skizzay/fsm/event.h"
#include <skizzay/utilz/traits.h>

namespace skizzay::event_sourcing {

template<class StreamIdType,
         class StreamVersionIntegral,
         class TimestampType>
class basic_snapshot_header {
   static_assert(utilz::equivalent_v<StreamIdType>,
         "StreamIdType must be equivalent");
   static_assert(utilz::is_time_point_v<TimestampType>,
         "TimestampType must be a std::chrono::time_point");

   StreamIdType stream_id_;
   basic_stream_version<StreamVersionIntegral> stream_sequence_;
   TimestampType timestamp_;

public:
   using stream_id_type = StreamIdType;
   using stream_version_type = basic_stream_version<StreamVersionIntegral>;
   using timestamp_type = TimestampType;

   constexpr basic_snapshot_header(StreamIdType sid, stream_version_type const ss,
         TimestampType ts) noexcept :
      stream_id_{std::move(sid)},
      stream_sequence_{ss},
      timestamp_{std::move(ts)}
   {
   }

   constexpr stream_id_type const &stream_id() const noexcept {
      return stream_id_;
   }

   constexpr stream_version_type stream_sequence() const noexcept {
      return stream_sequence_;
   }

   constexpr timestamp_type timestamp() const noexcept {
      return timestamp_;
   }
};

}
