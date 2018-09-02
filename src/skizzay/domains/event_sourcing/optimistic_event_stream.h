#pragma once

#include "skizzay/domains/event_sourcing/commit_header.h"
#include "skizzay/domains/event_sourcing/commit_result.h"
#include "skizzay/domains/event_sourcing/commit_store.h"
#include "skizzay/domains/event_sourcing/traits.h"
#include <cstdint>
#include <utility>

namespace skizzay::domains::event_sourcing {

template<class CommitStoreType, class EventCollectionType>
struct optimistic_event_stream {
   using stream_id_type = details_::typename_stream_id_type<CommitStoreType>;
   using commit_id_type = details_::typename_commit_id_type<CommitStoreType>;
   using timestamp_type = details_::typename_timestamp_type<CommitStoreType>;
   using event_type = typename EventCollectionType::value_type;
   using clock_type = typename timestamp_type::clock;

private:
   stream_id_type stream_id_;
   EventCollectionType committed_events_;
   EventCollectionType uncommitted_events_;
   CommitStoreType commits_;
   std::uint32_t stream_sequence_ = 0;
   std::uint32_t commit_sequence_ = 0;

   template <class CommitType>
   constexpr std::enable_if_t<is_basic_commit_v<CommitType>> save_committed_events(
         std::uint32_t const min_sequence, std::uint32_t const max_sequence, CommitType const &c) {
      commit_sequence_ = c.commit_sequence();
      std::uint32_t current_sequence = c.stream_sequence() - c.size() + 1;
      committed_events_.reserve(committed_events_.size() + c.size());
      for (auto const &evt : c) {
         if (current_sequence > max_sequence) {
            break;
         }
         else if (current_sequence++ >= min_sequence) {
            committed_events_.push_back(evt);
            stream_sequence_ = current_sequence - 1;
         }
      }
   }

   template <class CommitRange>
   constexpr std::enable_if_t<utilz::is_range_v<CommitRange>> save_committed_events(
         std::uint32_t const min_sequence, std::uint32_t const max_sequence, CommitRange const &c) {
      for (auto const &cmt : c) {
         save_committed_events(min_sequence, max_sequence, c);
      }
   }

   commit_result save_changes(commit_id_type commit_id) {
      using EventRange = decltype(to_range(uncommitted_events_));
      using CommitAttemptType = basic_commit_attempt<stream_id_type, commit_id_type, timestamp_type, EventRange>;
      using CommitType = basic_commit<stream_id_type, commit_id_type, timestamp_type, EventRange>;

      basic_commit_header header {
         stream_id_,
         stream_sequence_ + uncommitted_events_.size(),
         std::move(commit_id),
         commit_sequence_ + 1,
         clock_type::now()
      };

      std::uint32_t const next_stream_sequence = stream_sequence_ + 1;
      std::uint32_t const max_stream_sequence = std::numeric_limits<std::uint32_t>::max();
      CommitType result = commits_.put(CommitAttemptType{header, {to_range(uncommitted_events_), uncommitted_events_.size()}});
      switch (result.result_value()) {
         case commit_result::success:
            save_committed_events(next_stream_sequence, result.stream_sequence(), result);
            break;

         case commit_result::concurrency_collision:
            save_committed_events(next_stream_sequence, max_stream_sequence,
                  commits_.get(stream_id_, next_stream_sequence, max_stream_sequence));
            break;
      }
      return result.result_value();
   }

public:
   constexpr optimistic_event_stream(stream_id_type const &sid, CommitStoreType &&cs) noexcept :
      stream_id_{sid},
      committed_events_{},
      uncommitted_events_{},
      commits_{std::forward<CommitStoreType>(cs)}
   {
   }

   constexpr optimistic_event_stream(stream_id_type const &sid, CommitStoreType &&cs,
         std::uint32_t const min_sequence, std::uint32_t const max_sequence) noexcept :
      stream_id_{sid},
      committed_events_{},
      uncommitted_events_{},
      commits_{std::forward<CommitStoreType>(cs)}
   {
      save_committed_events(min_sequence, max_sequence,
            commits_.get(stream_id_, min_sequence, max_sequence));
   }

   constexpr stream_id_type const &stream_id() const noexcept {
      return stream_id_;
   }

   constexpr std::uint32_t stream_sequence() const noexcept {
      return stream_sequence_;
   }

   constexpr std::uint32_t commit_sequence() const noexcept {
      return commit_sequence_;
   }

   constexpr range_holder<EventCollectionType const &> committed_events() const noexcept {
      return {committed_events_, committed_events_.size()};
   }

   constexpr range_holder<EventCollectionType const &> uncommitted_events() const noexcept {
      return {uncommitted_events_, uncommitted_events_.size()};
   }

   template<class EventIterator>
   constexpr void put(EventIterator const b, EventIterator const e) {
      uncommitted_events_.reserve(std::distance(b, e) + uncommitted_events_.size());
      uncommitted_events_.insert(std::end(uncommitted_events_), b, e);
   }

   constexpr void put(event_type const &evt) {
      put(&evt, &evt + 1);
   }

   constexpr commit_result commit(commit_id_type commit_id) {
      if (uncommitted_events_.empty()) {
         return commit_result::no_pending_events;
      }
      else {
         return save_changes(std::move(commit_id));
      }
   }
};

}
