#pragma once

#include <skizzay/domains/inmemory/event_stream.h>
#include <skizzay/domains/event_stream_factory.h>
#include <skizzay/domains/validated_event_range.h>
#include <iterator>

namespace skizzay::domains::inmemory {

namespace details_ {
template<typename Mutex>
struct make_shared_mutex_impl {
   using type = Mutex;
};

template<>
struct make_shared_mutex_impl<std::mutex> {
   using type = std::shared_mutex;
};

template<typename Mutex>
using make_shared_mutex_t = typename make_shared_mutex_impl<Mutex>::type;
}

template<typename Event, typename CommitIdProvider, typename CommitTimestampProvider, typename Mutex=std::shared_mutex>
   requires concepts::event<Event>
      && std::invocable<CommitIdProvider> && concepts::identifier<std::invoke_result_t<CommitIdProvider>>
      && std::invocable<CommitTimestampProvider> && concepts::timestamp<std::invoke_result_t<CommitTimestampProvider>>
      && std::same_as<std::invoke_result_t<CommitTimestampProvider>, event_stream_timestamp_t<Event>>
class event_stream_store {
   using key_type = event_stream_id_t<Event>;
   using event_stream_state_type = details_::event_stream_state<Event, details_::make_shared_mutex_t<Mutex>>;
   using value_type = std::shared_ptr<event_stream_state_type>;
   using event_stream_table = std::pmr::unordered_map<key_type, value_type>;

   [[no_unique_address]] mutable Mutex mutex_;
   CommitIdProvider commit_id_provider_;
   CommitTimestampProvider commit_timestamp_provider_;
   event_stream_table event_streams_;

   constexpr std::invoke_result_t<CommitIdProvider> commit_id() noexcept(
      std::is_nothrow_invocable_v<CommitIdProvider>
   ) {
      return std::invoke(commit_id_provider_);
   }

   constexpr std::invoke_result_t<CommitTimestampProvider> commit_timestamp() noexcept(
      std::is_nothrow_invocable_v<CommitTimestampProvider>
   ) {
      return std::invoke(commit_timestamp_provider_);
   }

   value_type get_event_stream_state(key_type const &key) const {
      std::shared_lock lock{mutex_};
      auto const state = event_streams_.find(key);
      if (state == event_streams_.end()) {
         return std::allocate_shared<event_stream_state_type>(
            event_streams_.get_allocator(),
            key,
            event_streams_.get_allocator().resource()
         );
      }
      else {
         return state->second;
      }
   }

   value_type get_event_stream_state(key_type const &key) {
      auto const state = event_streams_.find(key);
      if (state == event_streams_.end()) {
         return std::allocate_shared<event_stream_state_type>(
            event_streams_.get_allocator(),
            key,
            event_streams_.get_allocator().resource()
         );
      }
      else {
         return state->second;
      }
   }

   constexpr auto validate(precommit<event_stream_id_t<Event>, event_stream_sequence_t<Event>> const &p) const noexcept {
      return ensure_event_id_matches(p.event_stream_id())
            | ensure_event_is_sequenced(p.precommit_sequence().next());
   }

   constexpr auto apply_timestamp(event_stream_timestamp_t<Event> const &timestamp) const noexcept {
      return std::ranges::views::transform([&timestamp](Event e) {
         return with_timestamp(e, timestamp);
      });
   }

   constexpr void persist_changes(value_type state) {
      event_stream_id_t<Event> id = state->event_stream_id_;
      auto [i, result] = event_streams_.try_emplace(std::move(id), std::move(state));

      if (!result) {
         i->second = std::move(state);
      }
   }

   constexpr void rollback_to(event_stream_state_type &state, event_stream_sequence_t<Event> target) {
      if (state.events_.size() > target.value()) {
         state.events_.erase(state.events_.begin() + target.value(), state.events_.end());
      }
   }

public:
   using event_stream_type = event_stream<Event, Mutex>;

   explicit event_stream_store(
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

   event_stream_type get_event_stream(key_type const &event_stream_id) const {
      return event_stream_type{get_event_stream_state(event_stream_id)};
   }

   template<concepts::event_range_of<Event> EventRange>
   auto put_event_stream(event_stream_type stream, EventRange &&range) {
      precommit p{event_stream_id(stream), event_stream_sequence(stream)};
      std::scoped_lock lock{mutex_};
      auto timestamp{commit_timestamp()};
      try {
         std::ranges::copy(
            ensure_nonempty(std::forward<EventRange>(range))
                  | validate(p)
                  | apply_timestamp(timestamp),
            std::back_inserter(stream.state_->events_)
         );
         auto new_sequence{event_stream_sequence(stream)};
         persist_changes(std::move(stream.state_));
         return p.commit_success(commit_id(), std::move(timestamp), new_sequence.value() - p.precommit_sequence().value());
      }
      catch (...) {
         rollback_to(*stream.state_, p.precommit_sequence());
         return p.commit_error(commit_id(), std::move(timestamp), std::current_exception());
      }
   }
};

}