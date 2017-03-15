#pragma once

#include <array>
#include <cstdint>
#include <system_error>
#include <type_traits>

namespace domains {

template <class EventSourceImpl>
class event_source final : public EventSourceImpl {
public:
   using EventSourceImpl::num_events;
   using EventSourceImpl::save;

   template <class AggregateIdType, class EventRouter>
   void load_events(AggregateIdType const aggregate_id, EventRouter &route) const noexcept {
      load_events(aggregate_id, std::size_t{0U}, route);
   }

   template <class AggregateIdType, class EventRouter>
   void load_events(AggregateIdType const aggregate_id, std::size_t const start_event_num,
                    EventRouter &route) const noexcept {
      for (auto const event : impl().get_events(start_event_num, aggregate_id)) {
         route(event);
      }
   }

private:
   EventSourceImpl &impl() noexcept {
      return *static_cast<EventSourceImpl *>(this);
   }

   EventSourceImpl const &impl() const noexcept {
      return *static_cast<EventSourceImpl const *>(this);
   }
};

class null_event_source {
   friend class event_source<null_event_source>;

   template <class AggregateIdType>
   std::size_t num_events(AggregateIdType const) const noexcept {
      return std::size_t{0U};
   }

   template <class Event>
   std::error_code save(Event const &) noexcept {
      return {};
   }

   template <class AggregateIdType>
   std::array<char, 0> get_events(std::size_t const, AggregateIdType const) const noexcept {
      return {};
   }
};
}
