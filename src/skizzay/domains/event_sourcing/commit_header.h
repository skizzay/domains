#pragma once

#include "skizzay/domains/event_sourcing/commit_sequence.h"
#include "skizzay/domains/event_sourcing/stream_version.h"
#include <skizzay/utilz/traits.h>
#include <cstdint>
#include <utility>

namespace skizzay::domains::event_sourcing {

template<class StreamIdType,
         class StreamVersionIntegral,
         class CommitIdType,
         class CommitSequenceIntegral,
         class TimestampType>
class basic_commit_header {
   static_assert(utilz::equivalent_v<StreamIdType>,
         "StreamIdType must be equivalent");
   static_assert(utilz::equivalent_v<CommitIdType>,
         "CommitIdType must be equivalent");
   static_assert(utilz::is_time_point_v<TimestampType>,
         "TimestampType must be a std::chrono::time_point");

   StreamIdType stream_id_;
   basic_stream_version<StreamVersionIntegral> stream_sequence_;
   CommitIdType commit_id_;
   basic_commit_sequence<CommitSequenceIntegral> commit_sequence_;
   TimestampType timestamp_;

public:
   using stream_id_type = StreamIdType;
   using stream_version_type = basic_stream_version<StreamVersionIntegral>;
   using commit_id_type = CommitIdType;
   using commit_sequence_type = basic_commit_sequence<CommitSequenceIntegral>;
   using timestamp_type = TimestampType;

   constexpr basic_commit_header(StreamIdType sid, stream_version_type const ss,
                                 CommitIdType cid, commit_sequence_type const cs, 
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

   constexpr stream_version_type stream_sequence() const noexcept {
      return stream_sequence_;
   }

   constexpr commit_id_type const &commit_id() const noexcept {
      return commit_id_;
   }

   constexpr commit_sequence_type commit_sequence() const noexcept {
      return commit_sequence_;
   }

   constexpr timestamp_type timestamp() const noexcept {
      return timestamp_;
   }
};

}
