#pragma once

#include <skizzay/domains/inmemory/event_stream.h>

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
class event_stream_factory {
   using key_type = event_stream_id_t<Event>;
   using event_stream_state_type = details_::event_stream_state<Event, CommitIdProvider, CommitTimestampProvider, details_::make_shared_mutex_t<Mutex>>;
   using value_type = std::shared_ptr<event_stream_state_type>;
   using event_stream_table = std::pmr::unordered_map<key_type, value_type>;

   [[no_unique_address]] mutable Mutex mutex_;
   CommitIdProvider commit_id_provider_;
   CommitTimestampProvider commit_timestamp_provider_;
   event_stream_table event_streams_;

   value_type get_event_stream_state(key_type const &key) {
      std::shared_lock lock{mutex_};
      auto const state = event_streams_.find(key);
      if (state == event_streams_.end()) {
         return std::allocate_shared<event_stream_state_type>(
            event_streams_.get_allocator(),
            key,
            event_streams_.get_allocator().resource(),
            commit_id_provider_,
            commit_timestamp_provider_
         );
      }
      else {
         return state->second;
      }
   }

public:
   using event_stream_type = event_stream<Event, CommitIdProvider, CommitTimestampProvider, Mutex>;

   explicit event_stream_factory(
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