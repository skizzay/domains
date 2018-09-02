#pragma once

#include <skizzay/utilz/traits.h>
#include <cstdint>
#include <utility>

namespace skizzay::domains::event_sourcing {

template<class StreamIdType,
         class CommitIdType,
         class TimestampType>
class basic_commit_header {
   static_assert(utilz::equivalent_v<StreamIdType>,
         "StreamIdType must be equivalent");
   static_assert(utilz::equivalent_v<CommitIdType>,
         "CommitIdType must be equivalent");
   static_assert(utilz::is_time_point_v<TimestampType>,
         "TimestampType must be a std::chrono::time_point");

   StreamIdType stream_id_;
   std::uint32_t stream_sequence_;
   CommitIdType commit_id_;
   std::uint32_t commit_sequence_;
   TimestampType timestamp_;

public:
   using stream_id_type = StreamIdType;
   using commit_id_type = CommitIdType;
   using timestamp_type = TimestampType;

   constexpr basic_commit_header(StreamIdType sid, std::uint32_t const ss,
                                 CommitIdType cid, std::uint32_t const cs, 
                                 TimestampType ts) noexcept :
      stream_id_{std::move(sid)},
      stream_sequence_{ss},
      commit_id_{std::move(cid)},
      commit_sequence_{cs},
      timestamp_{std::move(ts)}
   {
   }

   constexpr stream_id_type const &stream_id() const noexcept {
      return stream_id_;
   }

   constexpr std::uint32_t stream_sequence() const noexcept {
      return stream_sequence_;
   }

   constexpr commit_id_type const &commit_id() const noexcept {
      return commit_id_;
   }

   constexpr std::uint32_t commit_sequence() const noexcept {
      return commit_sequence_;
   }

   constexpr timestamp_type const &timestamp() const noexcept {
      return timestamp_;
   }
};

}
