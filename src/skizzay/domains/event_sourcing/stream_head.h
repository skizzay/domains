#pragma once

#include <skizzay/utilz/traits.h>
#include <cstdint>
#include <utility>

namespace skizzay::event_sourcing {

template<class StreamIdType>
class stream_head {
   static_assert(utilz::equivalent_v<StreamIdType>,
         "StreamIdType must be equivalent");

   StreamIdType stream_id_;
   std::uint32_t stream_sequence_;
   std::uint32_t snapshot_sequence_;

public:
   using stream_id_type = StreamIdType;

   constexpr stream_head(stream_id_type sid, std::uint32_t const s, std::uint32_t const ss) noexcept :
      stream_id_{std::move(sid)},
      stream_sequence_{s},
      snapshot_sequence_{ss}
   {
   }

   constexpr stream_id_type const &stream_id() const noexcept {
      return stream_id_;
   }

   constexpr std::uint32_t stream_sequence() const noexcept {
      return stream_sequence_;
   }

   constexpr std::uint32_t snapshot_sequence() const noexcept {
      return snapshot_sequence_;
   }
};

template<class StreamIdType>
stream_head(StreamIdType const &, std::uint32_t, std::uint32_t) -> stream_head<StreamIdType>;

}
