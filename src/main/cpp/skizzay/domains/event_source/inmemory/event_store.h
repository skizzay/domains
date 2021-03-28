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

namespace details_ {
template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
struct event_stream_state : std::enable_shared_from_this<event_stream_state<Event, CommitIdProvider, CommitTimestampProvider>> {
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

}

template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
   requires concepts::event<Event>
      && std::invocable<CommitIdProvider> && skizzay::domains::concepts::identifier<std::invoke_result_t<CommitIdProvider>>
      && std::invocable<CommitTimestampProvider> && skizzay::domains::concepts::timestamp<std::invoke_result_t<CommitTimestampProvider>>
      && std::same_as<std::invoke_result_t<CommitTimestampProvider>, event_stream_timestamp_t<Event>>
class event_stream {
   using event_id_type = event_stream_id_t<Event>;
   using event_stream_sequence_type = event_stream_sequence_t<Event>;

public:
   using event_type = Event;
   using commit_type = basic_commit<std::invoke_result_t<CommitIdProvider>, event_id_type, event_stream_sequence_type, event_stream_timestamp_t<event_type>>;

   explicit event_stream(std::shared_ptr<details_::event_stream_state<Event, CommitIdProvider, CommitTimestampProvider>> state) noexcept :
      state_{std::move(state)}
   {
   }

   event_id_type event_stream_id() const noexcept {
      return state_->event_stream_id_;
   }

   generator<event_type const> events(event_stream_sequence_t<Event> exclusive_begin, event_stream_sequence_t<Event> inclusive_end) const {
      std::shared_lock lock{state_->events_mutex_};
      auto const &events = state_->events_;
      auto const b = std::begin(events);
      auto const e = std::next(b, std::min(inclusive_end.next().value(), events.size()));
      for (auto i = std::next(b, std::min(exclusive_begin.value(), events.size())); i != e; ++i) {
         co_yield *i;
      }
   }

   template<typename EventRange>
   requires concepts::event_range<EventRange>
      && std::same_as<event_stream_id_t<EventRange>, event_stream_id_t<Event>>
      && std::same_as<event_stream_sequence_t<EventRange>, event_stream_sequence_t<Event>>
      && std::same_as<event_stream_timestamp_t<EventRange>, event_stream_timestamp_t<Event>>
   commit_type put_events(EventRange &&events) {
      std::scoped_lock lock{state_->events_mutex_};
      auto const result = prepare_commit_result();
      try {
         validate(events, result.precommit_sequence);
         persist(events, result.commit_timestamp);
         return result.success(last_commited_sequence());
      }
      catch (...) {
         rollback_to(result.precommit_sequence);
         return result.error(std::current_exception());
      }
   }

   bool empty() const noexcept {
      std::shared_lock lock{state_->events_mutex_};
      return state_->events_.empty();
   }

private:
   std::shared_ptr<details_::event_stream_state<Event, CommitIdProvider, CommitTimestampProvider>> state_;

   struct commit_result final {
      std::invoke_result_t<CommitIdProvider> commit_id;
      std::invoke_result_t<CommitTimestampProvider> commit_timestamp;
      event_stream_id_t<Event> event_stream_id;
      event_stream_sequence_t<Event> precommit_sequence;

      commit_type success(event_stream_sequence_t<Event> commit_sequence) const noexcept {
         return commit_type{
            commit_id,
            event_stream_id,
            commit_timestamp,
            precommit_sequence.next(),
            commit_sequence
         };
      }

      commit_type error(std::exception_ptr e) const noexcept {
         return commit_type{
            commit_id,
            event_stream_id,
            commit_timestamp,
            e
         };
      }
   };

   commit_result prepare_commit_result() noexcept {
      return commit_result{
         std::invoke(state_->commit_id_provider_),
         std::invoke(state_->commit_timestamp_provider_),
         event_stream_id(),
         last_commited_sequence()
      };
   }

   event_stream_sequence_type last_commited_sequence() const noexcept {
      // Only safe to do under a lock
      return event_stream_sequence_type{state_->events_.size()};
   }

   template<typename EventRange>
   void validate(EventRange const &events, event_stream_sequence_t<Event> last_sequence) const {
      if (std::ranges::empty(events)) {
         throw std::range_error{"Empty event range provided"};
      }
      else {
         std::ranges::for_each(events, [last_sequence, expected_event_stream_id=this->event_stream_id()](auto const &event) mutable {
            if (skizzay::domains::event_source::event_stream_id(event) != expected_event_stream_id) {
               throw std::invalid_argument{"Event stream id does match the event's id"};
            }
            else if (skizzay::domains::event_source::event_stream_sequence(event) != last_sequence.next()) {
               throw concurrency_collision{"Expected event stream sequence was not found"};
            }
            else {
               last_sequence = last_sequence.next();
            }
         });
      }
   }

   template<typename EventRange>
   void persist(EventRange &&events, event_stream_timestamp_t<Event> commit_timestamp) {
      auto timestamped_events = std::forward<EventRange>(events)
         | std::views::transform([commit_timestamp](auto &&event) {
            return skizzay::domains::event_source::with_timestamp(event, commit_timestamp);
         });
      std::ranges::copy(timestamped_events, std::back_inserter(state_->events_));
   }

   void rollback_to(event_stream_sequence_t<Event> const last_sequence) noexcept {
      auto &events = state_->events_;
      if (events.size() != last_sequence.value()) {
         events.erase(events.begin() + last_sequence.value(), events.end());
      }
   }
};


template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider>
   requires concepts::event<Event>
      && std::invocable<CommitIdProvider> && skizzay::domains::concepts::identifier<std::invoke_result_t<CommitIdProvider>>
      && std::invocable<CommitTimestampProvider> && skizzay::domains::concepts::timestamp<std::invoke_result_t<CommitTimestampProvider>>
      && std::same_as<std::invoke_result_t<CommitTimestampProvider>, event_stream_timestamp_t<Event>>
class event_store {
   using key_type = event_stream_id_t<Event>;
   using event_stream_state_type = details_::event_stream_state<Event, CommitIdProvider, CommitTimestampProvider>;
   using value_type = std::shared_ptr<event_stream_state_type>;
   using event_stream_table = std::pmr::unordered_map<key_type, value_type>;

   mutable std::mutex mutex_;
   CommitIdProvider commit_id_provider_;
   CommitTimestampProvider commit_timestamp_provider_;
   event_stream_table event_streams_;

   value_type allocate_event_stream_state(key_type const &key) {
      auto value = std::allocate_shared<event_stream_state_type>(
         event_streams_.get_allocator(),
         key,
         event_streams_.get_allocator().resource(),
         commit_id_provider_,
         commit_timestamp_provider_
      );
      event_streams_.emplace(key, value);
      return value;
   }

   value_type get_event_stream_state(key_type const &key) {
      std::scoped_lock lock{mutex_};
      auto const state = event_streams_.find(key);
      if (state == event_streams_.end()) {
         return allocate_event_stream_state(key);
      }
      else {
         return state->second;
      }
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
      commit_timestamp_provider_{std::move(commit_timestamp_provider)},
      event_streams_{std::move(alloc)}
   {
   }

   event_stream_type get_event_stream(key_type const &event_stream_id) {
      return event_stream_type{get_event_stream_state(event_stream_id)};
   }
};

}