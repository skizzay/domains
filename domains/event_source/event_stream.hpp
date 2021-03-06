#pragma once

#include <system_error>
#include <domains/event_source/event_range.hpp>

namespace domains {

template <typename T>
concept bool EventStream = requires(T const ct, T t) {
   { ct.id() } noexcept/* -> EqualityComparable */;
   { ct.version() } noexcept -> UnsignedIntegral;
   { ct.committed_events() } noexcept -> EventRange;
   { ct.uncommitted_events() } noexcept -> EventRange;
   //{ t.put(Event) } noexcept;
   { t.clear_changes() } noexcept;
   { t.commit_changes() } noexcept -> std::error_code;
};

template <EqualityComparable IdType = uint64_t,
          UnsignedIntegral VersionType = uint32_t, UnsignedIntegral CommitSequenceType=VersionType>
class event_stream {
   IdType id_;
   VersionType version_;
   CommitSequenceType sequence_;

public:
   using id_type = IdType;

   constexpr event_stream(
         IdType const id, VersionType const version,
         CommitSequenceType const sequence) noexcept :
      id_{id}, version_{version}, sequence_{sequence}
   {
   }

   constexpr IdType id() const noexcept {
      return id_;
   }
   constexpr VersionType version() const noexcept {
      return version_;
   }
   constexpr CommitSequenceType commit_sequence() const noexcept {
      return sequence_;
   }
#if 0
   auto committed_events() const;   // range of EncodedType
   auto uncommitted_events() const; // range of EncodedType

   std::error_code put(EncodedType const &) noexcept;
   std::error_code save_changes(id_type const commit_id) noexcept;
   void clear_changes() noexcept;
};

template <class CommitProvider>
class optimistic_event_stream : public event_stream<something, something> {
   std::experimental::pmr::vector<EventType> committed_events_;
   std::experimental::pmr::vector<EventType> uncommitted_events_;
   CommitProvider &commits;

   void populate(CommitRange cr) {
   }

public:
   std::error_code put(EncodedType const &event) noexcept {
      try {
         uncommitted_events_.push_back(event);
         return {};
      } catch (std::bad_alloc const &) {
         return make_error_code(std::errc::not_enough_memory);
      }
   }

   std::error_code save_changes(id_type const commit_id,
                                std::chrono::nanoseconds const &utc) noexcept {
      try {
         commit_attempt<id_type> attempt{commit_id, commit_sequence + 1, utc, stream_id,
                                         stream_version + uncommitted_events_.size()};
         commit<id_type> c = commits.try_put(attempt);
      } catch (std::exception const &) {
         return make_error_code(std::errc::not_enough_memory);
      }
   }

   void clear_changes() noexcept {
      uncommitted_events_.clear();
   }
#endif
};
}
