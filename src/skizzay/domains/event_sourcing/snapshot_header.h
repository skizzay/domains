#pragma once

#include "skizzay/domains/event_sourcing/traits.h"
#include "skizzay/fsm/event.h"
#include <skizzay/utilz/traits.h>

namespace skizzay::event_sourcing {

template<class StreamIdType,
         class TimestampType>
class basic_snapshot_header {
   static_assert(utilz::equivalent_v<StreamIdType>,
         "StreamIdType must be equivalent");
   static_assert(utilz::is_time_point_v<TimestampType>,
         "TimestampType must be a std::chrono::time_point");

   StreamIdType stream_id_;
   std::uint32_t stream_sequence_;
   TimestampType timestamp_;

public:
   using stream_id_type = StreamIdType;
   using timestamp_type = TimestampType;

   constexpr basic_snapshot_header(StreamIdType sid, std::uint32_t const ss,
         TimestampType ts) noexcept :
      stream_id_{std::move(sid)},
      stream_sequence_{ss},
      timestamp_{std::move(ts)}
   {
   }

   constexpr stream_id_type const &stream_id() const noexcept {
      return stream_id_;
   }

   constexpr std::uint32_t stream_sequence() const noexcept {
      return stream_sequence_;
   }

   constexpr timestamp_type const &timestamp() const noexcept {
      return timestamp_;
   }
};

}
