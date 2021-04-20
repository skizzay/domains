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

   constexpr event_stream_sequence_t<Event> event_stream_sequence() const noexcept {
      using sequence_value_type = typename event_stream_sequence_t<Event>::value_type;
      return event_stream_sequence_t<Event>{static_cast<sequence_value_type>(this->events_.size())};
   }

   constexpr event_stream_timestamp_t<Event> event_stream_timestamp() const noexcept {
      using skizzay::domains::event_stream_timestamp;
      return std::ranges::empty(events_)
         ? event_stream_timestamp_t<Event>{}
         : event_stream_timestamp(this->events_.back());
   }

   constexpr std::tuple<event_stream_id_t<Event>, event_stream_sequence_t<Event>, event_stream_timestamp_t<Event>> event_stream_head() const noexcept {
      return std::make_tuple(
         this->event_stream_id_,
         this->event_stream_sequence(),
         this->event_stream_timestamp()
      );
   }
};

}

template<concepts::event Event, typename Mutex>
class event_stream {
private:
   using event_id_type = event_stream_id_t<Event>;
   using event_stream_sequence_type = event_stream_sequence_t<Event>;
   using sequence_value_type = typename event_stream_sequence_type::value_type;

   template<typename, typename, typename, typename>
   friend class event_stream_store;

   std::shared_ptr<details_::event_stream_state<Event, Mutex>> state_;

   constexpr explicit event_stream(std::shared_ptr<details_::event_stream_state<Event, Mutex>> state) noexcept :
      state_{std::move(state)}
   {
   }

   constexpr std::ptrdiff_t advance_index(event_stream_sequence_type const &x) const noexcept {
      return std::max<std::ptrdiff_t>(0, std::min<std::ptrdiff_t>(x.value() - static_cast<sequence_value_type>(1), std::size(state_->events_)));
   }

   // NOTE: Validation has already occurred and lock has been acquired at this point
   template<concepts::event_range_of<Event> EventRange>
   constexpr void append_events(EventRange &&events) {
         std::ranges::copy(
            std::forward<EventRange>(events),
            std::back_inserter(state_->events_)
         );
   }

   constexpr void rollback_to(event_stream_sequence_t<Event> target_sequence) {
      using I = std::common_type_t<
         std::make_unsigned_t<decltype(state_->events_.size())>,
         std::make_unsigned_t<typename event_stream_sequence_t<Event>::value_type>
      >;
      if (static_cast<I const>(state_->events_.size()) > static_cast<I const>(target_sequence.value())) {
         state_->events_.erase(state_->events_.begin() + target_sequence.value(), state_->events_.end());
      }
   }

public:
   using event_type = Event;

   constexpr event_id_type event_stream_id() const noexcept {
      return state_->event_stream_id_;
   }

   constexpr event_stream_sequence_t<Event> event_stream_sequence() const noexcept {
      std::shared_lock lock{state_->events_mutex_};
      return state_->event_stream_sequence();
   }

   constexpr event_stream_timestamp_t<Event> event_stream_timestamp() const noexcept {
      using skizzay::domains::event_stream_timestamp;
      std::shared_lock lock{state_->events_mutex_};
      return state_->event_stream_timestamp();
   }

   constexpr std::tuple<event_id_type, event_stream_sequence_t<Event>, event_stream_timestamp_t<Event>> event_stream_head() const {
      std::shared_lock lock{state_->events_mutex_};
      return state_->event_stream_head();
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