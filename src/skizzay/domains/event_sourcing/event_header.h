#pragma once

#include "skizzay/domains/event_sourcing/identifier.h"
#include "skizzay/domains/event_sourcing/timestamp.h"
#include "skizzay/domains/event_sourcing/sequence.h"
#include <skizzay/utilz/traits.h>
#include <chrono>
#include <cstdint>

namespace skizzay::domains::event_sourcing {

template<concepts::identifier StreamId, concepts::stream_version StreamVersion, concepts::timestamp Timestamp>
class basic_event_header {
   StreamId stream_id_;
   StreamVersion stream_version_;
   Timestamp timestamp_;

public:
   using stream_id_type = StreamId;
   using stream_version_type = StreamVersion;
   using timestamp_type = Timestamp;

   constexpr basic_event_header(
         stream_id_type si,
         stream_version_type const sv,
         timestamp_type const ts) noexcept(std::is_nothrow_move_constructible_v<stream_id_type>) :
      stream_id_{std::move(si)},
      stream_version_{sv},
      timestamp_{ts}
   {
   }

   constexpr basic_event_header(stream_id_type si) noexcept(std::is_nothrow_move_constructible_v<stream_id_type>) :
      basic_event_header(std::move(si), unsequenced<typename stream_version_type::tag_type, typename stream_version_type::value_type>, timestamp_not_set<timestamp_type>)
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

namespace concepts {
   template<class T>
   concept event_header = utilz::is_template_v<T, basic_event_header>;
}

}