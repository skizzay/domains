#pragma once

#include "skizzay/domains/event_sourcing/event.h"
#include <cstdint>
#include <functional>
#include <type_traits>

namespace skizzay::domains {

template<class AggregateRootIdType, class FSM>
class aggregate_root {
   AggregateRootIdType id_;
   FSM fsm_;
   std::uint32_t version_;

protected:
   constexpr aggregate_root(AggregateRootIdType id, FSM fsm, std::uint32_t const v) noexcept :
      id_{std::move(id)},
      fsm_{std::move(fsm)},
      version_{v}
   {
   }

   constexpr aggregate_root(AggregateRootIdType id, FSM fsm) noexcept :
      aggregate_root{std::move(id), std::move(fsm)}
   {
   }

public:
   using id_type = AggregateRootIdType;
   using version_type = std::uint32_t;

   constexpr id_type const &id() const noexcept {
      return id_;
   }

   constexpr version_type version() const noexcept {
      return version_;
   }

   template<class Command>
   constexpr void validate(Command const &c) const {
      fsm_.query([&c](auto const &state) {
            state.validate(c);
         });
   }

   template<class Command>
   constexpr auto get_events(Command const &c) const noexcept {
      return fsm_.query([&c](auto const &state) {
            return state.get_events(c);
         });
   }

   template<class Clock, class Duration, class ...TagsAndEventData>
   constexpr basic_event<Clock, Duration, TagsAndEventData...> const &apply_change(basic_event<Clock, Duration, TagsAndEventData...> const &e) {
      fsm_.on(e);
      return e;
   }

   template<class EventRange>
   constexpr void load_from_history(EventRange const &events) {
      using std::begin;
      using std::end;
      using std::for_each;
      for_each(begin(events), end(events),
         [this](auto const &e) {
            ++version_;
            fsm_.on(e);
         });
   }
};

}
