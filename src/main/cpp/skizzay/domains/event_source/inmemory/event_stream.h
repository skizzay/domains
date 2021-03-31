#pragma once

#include <skizzay/domains/event_source/commit.h>
#include <skizzay/domains/event_source/concurrency_collision.h>
#include <skizzay/domains/event_source/event.h>
#include <skizzay/domains/event_source/event_stream.h>
#include <skizzay/domains/event_source/generator.h>
#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <type_traits>
#include <vector>

namespace skizzay::domains::event_source::inmemory {

namespace details_ {
template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider, typename Mutex>
struct event_stream_state : std::enable_shared_from_this<event_stream_state<Event, CommitIdProvider, CommitTimestampProvider, Mutex>> {
   [[no_unique_address]] mutable Mutex events_mutex_;
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

}

template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider, typename Mutex>
   requires concepts::event<Event>
      && std::invocable<CommitIdProvider> && skizzay::domains::concepts::identifier<std::invoke_result_t<CommitIdProvider>>
      && std::invocable<CommitTimestampProvider> && skizzay::domains::concepts::timestamp<std::invoke_result_t<CommitTimestampProvider>>
      && std::same_as<std::invoke_result_t<CommitTimestampProvider>, event_stream_timestamp_t<Event>>
class event_stream : public skizzay::domains::event_source::event_stream_base<event_stream<Event, CommitIdProvider, CommitTimestampProvider, Mutex>, Event> {
   using event_id_type = event_stream_id_t<Event>;
   using event_stream_sequence_type = event_stream_sequence_t<Event>;

   friend event_stream_base<event_stream<Event, CommitIdProvider, CommitTimestampProvider, Mutex>, Event>;

public:
   using event_type = Event;
   using commit_type = basic_commit<std::invoke_result_t<CommitIdProvider>, event_id_type, event_stream_sequence_type, event_stream_timestamp_t<event_type>>;

   explicit event_stream(std::shared_ptr<details_::event_stream_state<Event, CommitIdProvider, CommitTimestampProvider, Mutex>> state) noexcept :
      state_{std::move(state)}
   {
   }

   event_id_type event_stream_id() const noexcept {
      return state_->event_stream_id_;
   }

   generator<event_type const> events(event_stream_sequence_t<Event> inclusive_begin, event_stream_sequence_t<Event> exclusive_end) const {
      std::shared_lock lock{state_->events_mutex_};
      auto const b = std::begin(state_->events_);
      auto const e = std::next(b, advance_index(exclusive_end));
      for (auto i = std::next(b, advance_index(inclusive_begin)); i != e; ++i) {
         co_yield *i;
      }
   }

   bool empty() const noexcept {
      std::shared_lock lock{state_->events_mutex_};
      return state_->events_.empty();
   }

private:
   std::shared_ptr<details_::event_stream_state<Event, CommitIdProvider, CommitTimestampProvider, Mutex>> state_;
   // These will be populated when we prepare for put.
   std::optional<std::remove_cvref_t<std::invoke_result_t<CommitIdProvider>>> commit_id_;
   std::optional<event_stream_timestamp_t<Event>> commit_timestamp_;

   constexpr std::ptrdiff_t advance_index(event_stream_sequence_type const &x) const noexcept {
      return std::min<std::ptrdiff_t>(x.value() - 1, std::size(state_->events_));
   }
   
   Mutex & get_mutex() const noexcept {
      return state_->events_mutex_;
   }

   void prepare_for_put() {
      commit_id_.emplace(std::invoke(state_->commit_id_provider_));
      commit_timestamp_.emplace(std::invoke(state_->commit_timestamp_provider_));
   }

   decltype(auto) commit_id() const {
      return skizzay::domains::get_reference(commit_id_);
   }

   decltype(auto) commit_timestamp() const {
      return skizzay::domains::get_reference(commit_timestamp_);
   }

   event_stream_sequence_type last_committed_sequence() const noexcept {
      // Only safe to do under a lock
      return event_stream_sequence_type{state_->events_.size()};
   }

   template<typename EventRange>
   void persist(EventRange &&events) {
      auto update_event_timestamp = [commit_timestamp=commit_timestamp()](auto &&event) {
         return skizzay::domains::event_source::with_timestamp(event, commit_timestamp);
      };
      auto timestamped_events = std::forward<EventRange>(events)
         | std::views::transform(update_event_timestamp);
      std::ranges::copy(timestamped_events, std::back_inserter(state_->events_));
   }

   void rollback_to(event_stream_sequence_t<Event> const last_sequence) noexcept {
      auto &events = state_->events_;
      if (events.size() > last_sequence.value()) {
         events.erase(events.begin() + last_sequence.value(), events.end());
      }
   }
};

}