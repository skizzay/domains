#pragma once

#include <skizzay/domains/commit.h>
#include <skizzay/domains/concurrency_collision.h>
#include <skizzay/domains/event.h>
#include <skizzay/domains/event_stream.h>
#include <skizzay/domains/generator.h>
#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <type_traits>
#include <vector>

namespace skizzay::domains::inmemory {

namespace details_ {
template<typename Event, typename Mutex>
struct event_stream_state : std::enable_shared_from_this<event_stream_state<Event, Mutex>> {
   [[no_unique_address]] mutable Mutex events_mutex_;
   event_stream_id_t<Event> event_stream_id_;
   std::pmr::vector<Event> events_;

   explicit event_stream_state(
      event_stream_id_t<Event> event_stream_id,
      std::pmr::memory_resource *resource
   ) noexcept :
      events_mutex_{},
      event_stream_id_{std::move(event_stream_id)},
      events_{resource}
   {
   }
};

}

template<concepts::event Event, typename Mutex>
class event_stream {
private:
   using event_id_type = event_stream_id_t<Event>;
   using event_stream_sequence_type = event_stream_sequence_t<Event>;

   template<typename, typename, typename, typename>
   friend class event_stream_store;

   std::shared_ptr<details_::event_stream_state<Event, Mutex>> state_;

   constexpr explicit event_stream(std::shared_ptr<details_::event_stream_state<Event, Mutex>> state) noexcept :
      state_{std::move(state)}
   {
   }

   constexpr std::ptrdiff_t advance_index(event_stream_sequence_type const &x) const noexcept {
      return std::max<std::ptrdiff_t>(0, std::min<std::ptrdiff_t>(x.value() - 1, std::size(state_->events_)));
   }

public:
   using event_type = Event;

   constexpr event_id_type event_stream_id() const noexcept {
      return state_->event_stream_id_;
   }

   constexpr event_stream_sequence_t<Event> event_stream_sequence() const noexcept {
      std::shared_lock lock{state_->events_mutex_};
      return event_stream_sequence_t<Event>{state_->events_.size()};
   }

   generator<event_type const> events(event_stream_sequence_t<Event> inclusive_begin, event_stream_sequence_t<Event> exclusive_end) const {
      std::shared_lock lock{state_->events_mutex_};
      auto const b = std::begin(state_->events_);
      auto const e = std::next(b, advance_index(exclusive_end));
      for (auto i = std::next(b, advance_index(inclusive_begin)); i != e; ++i) {
         co_yield *i;
      }
   }
};

}