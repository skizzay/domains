#pragma once

#include <domains/event_source/event_range.hpp>

namespace domains {
template <typename T>
concept bool Commit = requires(T t) {
   { t.id() } const noexcept -> EqualityComparable;
   { t.stream_id() } const noexcept -> EqualityComparable;
   { t.timestamp() } const noexcept -> TimePoint;
   { t.commit_sequence() } const noexcept -> UnsignedIntegral;
   { t.version() } const noexcept -> UnsignedIntegral;
   { t.events() } const noexcept -> EventRange;
};

template <EqualityComparable CommitIdType = std::uint64_t,
          EqualityComparable StreamIdType = std::uint64_t,
          TimePoint TimestampType = std::chrono::nanoseconds,
          UnsignedIntegral CommitSequenceType = std::uint32_t,
          UnsignedIntegral StreamVersionType = std::uint32_t>
class commit {
   CommitIdType id_;
   CommitSequenceType commit_sequence_;
   StreamIdType stream_id_;
   StreamVersionType stream_version_;
   TimestampType timestamp_;

public:
   constexpr commit(
         CommitIdType cid, CommitSequenceType const cs, StreamIdType sid, StreamVersionType const sv,
         TimestampType const ts) noexcept :
      id_{std::move(cid)},
      commit_sequence_{cs},
      stream_id_{std::move(sid)},
      stream_version_{sv},
      timestamp_{ts}
   {
   }

   constexpr auto id() const noexcept {
      return id_;
   }

   constexpr auto sequence() const noexcept {
      return commit_sequence_;
   }

   constexpr auto stream_id() const noexcept {
      reutrn stream_id_;
   }

   constexpr auto stream_version() const noexcept {
      return stream_version_;
   }

   constexpr auto timestamp() const noexcept {
      return timestamp_;
   }
};

}
