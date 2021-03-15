#pragma once

#include <skizzay/domains/event_source/commit.h>
#include <skizzay/domains/event_source/concurrency_collision.h>
#include <skizzay/domains/event_source/event.h>
#include <skizzay/domains/event_source/generator.h>
#include <algorithm>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace skizzay::domains::event_source::inmemory {


template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
class event_store;

namespace details_ {
template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
struct event_stream_state : std::enable_shared_from_this<event_stream_state<Event>> {
   mutable std::shared_mutex events_mutex_;
   event_stream_id_t<Event> event_stream_id_;
   std::pmr::vector<Event> events_;
   CommitIdProvider commit_id_provider_;
   CommitTimestampProvider commit_timestamp_provider_;

   explicit event_stream_state(
      event_stream_id_t<Event> event_stream_id,
      std::pmr::memory_resource *resource,
      CommitIdProvider commit_id_provider,
      CommitTimestampProvider commit_timestamp_provider
   ) noexcept :
      events_mutex_{},
      event_stream_id_{std::move(event_stream_id)},
      events_{resource},
      commit_id_provider_{std::move(commit_id_provider)},
      commit_timestamp_provider_{std::move(commit_timestamp_provider)}
   {
   }
};

template<typename StreamId, typename Sequence, typename Timestamp>
struct validate_and_apply_timestamp {
   StreamId event_stream_id_;
   Sequence last_seen_sequence_;
   Timestamp event_timestamp_;

   template<typename Event>
      requires concepts::event<Event>
         && std::is_same_v<Timestamp, event_stream_timestamp_t<Event>>
         && std::is_same_v<Sequence, event_stream_sequence_t<Event>>
   decltype(auto) operator()(Event &e) const {
      if (skizzay::domains::event_source::event_stream_id(e) != event_stream_id_) {
         throw std::invalid_argument{"Event stream id does match within the event itself"};
      }
      else if (skizzay::domains::event_source::event_stream_sequence(e) != last_seen_sequence_.next()) {
         throw concurrency_collision{"Expected event stream sequence was not found"};
      }
      else {
         last_seen_sequence_ = skizzay::domains::event_source::event_stream_sequence(e);
         return skizzay::domains::event_source::with_timestamp(e, event_timestamp_);
      }
   }
};

}

template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
   requires concepts::event<Event>
      && std::invocable<CommitIdProvider> && concepts::identifier<std::invoke_result_t<CommitIdProvider>>
      && std::invocable<CommitTimestampProvider> && concepts::timestamp<std::invoke_result_t<CommitTimestampProvider>>
      && std::is_same_v<std::invoke_result_t<CommitTimestampProvider>, event_stream_timestamp_t<Event>>
class event_stream {
   using event_id_type = event_id_t<Event>;
   using event_stream_sequence_type = event_stream_sequence_t<Event>;

public:
   using event_type = Event;
   using commit_type = basic_commit<std::invoke_result_t<CommitIdProvider>, event_type>

   event_id_type event_stream_id() const noexcept {
      return state_->event_stream_id_;
   }

   generator<event_type> events(event_stream_sequence_type const exclusive_begin, event_stream_sequence_type const inclusive_end) const {
      std::shared_lock lock{state_->events_mutex_};
      auto const &events = state_->events_;
      auto const b = std::begin(events);
      auto const e = std::advance(b, std::min(inclusive_end.next().value(), events.size()));
      for (auto i = std::advance(b, std::min(exclusive_begin.value(), events.size())); i != b; ++i) {
         co_yield *i;
      }
   }

   template<concepts::event_range<event_type> EventRange>
   commit_type put_events(EventRange events) {
      auto commit_id = std::invoke(state_->commit_id_provider_);
      // Should we capture the timestamp on the other side of the lock??
      auto commit_timestamp = std::invoke(state_->commit_timestamp_provider_);
      std::scoped_lock lock{state_->events_mutex_};
      auto const last_sequence = state_->events_.size();
      try {
         if (std::ranges::empty(events)) {
            throw std::invalid_arguments{"Event range cannot be empty for a commit"};
         }
         auto timestamped_events = events | std::views::transform(details_::validate_and_apply_timestamp(
            event_stream_id(),
            event_stream_sequence_type{last_sequence},
            commit_timestamp
         ));
         std::ranges::copy(timestamped_events, std::ranges::end(state_->events_));
         return commit_type{
            commit_id,
            std::ranges::begin(state_->events_) + last_sequence + 1,
            std::ranges::end(state_->events_)
         };
      }
      catch (...) {
         rollback_to(last_sequence);
         return commit_type{
            commit_id,
            commit_timestamp,
            std::current_exception()
         };
      }
   }

   bool empty() const noexcept {
      std::shared_lock lock{state_->events_mutex_};
      return state_->events_.empty();
   }

private:
   template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
   friend class event_store;

   std::shared_ptr<details_::event_stream_state<Event>> state_;

   template<concepts::event_range<event_type> EventRange>
   void validate(EventRange events) const noexcept {
      auto const b = std::ranges::begin(events);
      if (b == std::ranges::end(events)) {
         throw std::range_error{"Empty event range provided"};
      }
   }

   void rollback_to(std::integral const last_sequence) {
      auto &events = state_->events_;
      if (events.size() != last_sequence) {
         events.erase(events.begin() + last_sequence, events.end())
      }
   }
};


template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
   requires concepts::event<Event>
      && std::invocable<CommitIdProvider> && concepts::identifier<std::invoke_result_t<CommitIdProvider>>
      && std::invocable<CommitTimestampProvider> && concepts::timestamp<std::invoke_result_t<CommitTimestampProvider>>
      && std::is_same_v<std::invoke_result_t<CommitTimestampProvider>, event_stream_timestamp_t<Event>>
class event_store {
   using key_type = event_stream_id_t<Event>;
   using event_stream_state_type = details_::event_stream_state<Event, CommitIdProvider, CommitTimestampProvider>
   using value_type = std::shared_ptr<event_stream_state_type>;
   using event_stream_table = std::pmr::unordered_map<key_type, value_type>;

   std::mutex mutex_;
   CommitIdProvider commit_id_provider_;
   CommitTimestampProvider commit_timestamp_provider_;
   event_stream_table event_streams_;

   value_type allocate_event_stream_state(key_type key) {
      auto value = std::allocate_shared<event_stream_state_type>(
         event_streams_.allocator(),
         key,
         event_streams_.allocator().resource(),
         commit_id_provider_,
         commit_timestamp_provider_
      );
      event_streams_.emplace(std::move(key), value);
      return value;
   }

public:
   using event_stream_type = event_stream<Event, CommitIdProvider, CommitTimestampProvider>;

   explicit event_store(
      CommitIdProvider commit_id_provider,
      CommitTimestampProvider commit_timestamp_provider,
      typename event_stream_table::allocator_type alloc={}
   ) noexcept :
      mutex_{},
      commit_id_provider_{std::move(commit_id_provider)},
      commit_timestamp_provider_{std::move(commit_id_provider)},
      event_streams_{std::move(alloc)}
   {
   }

   event_stream_type create_event_stream(key_type event_stream_id) {
      std::scoped_lock lock{mutex_};
      auto const state = event_streams_.find(event_stream_id);
      if (state == event_streams_.end()) {
         return {allocate_event_stream_state(std::move(event_stream_id))};
      }
      else {
         event_stream result{state->second};
         if (result.empty()) {
            return result;
         }
         else {
            throw std::logic_error{"Event stream already exists"};
         }
      }
   }

   std::optional<event_stream_type> get_event_stream(key_type const &event_stream_id) const {
      std::scoped_lock lock{mutex_};
      auto const state = event_streams_.find(event_stream_id);
      if (state == event_streams_.end()) {
         return std::nullopt;
      }
      else {
         event_stream result{state->second};
         if (result.empty()) {
            return std::nullopt;
         }
         else {
            return result;
         }
      }
   }
};

}