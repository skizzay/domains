#pragma once

#include <skizzay/domains/inmemory/event_stream.h>
#include <skizzay/domains/commit.h>
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

   constexpr auto apply_timestamp(event_stream_timestamp_t<Event> const &timestamp) const noexcept {
      return std::ranges::views::transform([&timestamp](Event e) {
         return with_timestamp(e, timestamp);
      });
   }

   template<concepts::event_range_of<Event> EventRange>
   constexpr void persist_changes(event_stream<Event, Mutex> stream, EventRange &&events) {
      event_stream_id_t<Event> id = event_stream_id(stream);
      stream.append_events(events | apply_timestamp(commit_timestamp()));
      std::scoped_lock lock{mutex_};
      auto [i, result] = event_streams_.try_emplace(std::move(id), std::move(stream.state_));

      if (!result) {
         i->second = std::move(stream.state_);
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
   auto put_event_stream(event_stream_type stream, EventRange &range) {
      std::scoped_lock stream_lock{stream.state_->events_mutex_};
      auto head = stream.state_->event_stream_head();
      try {
         persist_changes(stream, validate_commit_range(head, range));
         return commit_success(commit_id(), head, stream.state_->event_stream_head());
      }
      catch (...) {
         stream.rollback_to(event_stream_sequence(head));
         return commit_error(commit_id(), head, commit_timestamp(), std::current_exception());
      }
   }
};

}