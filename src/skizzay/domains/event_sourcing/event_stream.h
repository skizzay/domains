#pragma once

#include "skizzay/domains/event_sourcing/traits.h"
#include <cstdint>

namespace skizzay::domains::event_sourcing {

template<class T>
class event_stream {
   static_assert(is_event_stream_v<T>, "T must be an event stream");
   T t_;

public:
    using stream_id_type = typename utilz::strip_reference_wrapper_t<T>::stream_id_type;
    using commit_id_type = typename utilz::strip_reference_wrapper_t<T>::commit_id_type;
    using timestamp_type = typename utilz::strip_reference_wrapper_t<T>::timestamp_type;

    constexpr event_stream(T t) noexcept :
       t_{t}
    {
    }

    constexpr decltype(auto) stream_id() const noexcept {
       return t_.stream_id();
    }

    constexpr decltype(auto) uncommited_events() const noexcept {
       return t_.uncommited_events();
    }

    constexpr decltype(auto) commited_events() const noexcept {
       return t_.commited_events();
    }

    constexpr std::uint32_t stream_sequence() const noexcept {
       return t_.stream_sequence();
    }

    constexpr std::uint32_t commit_sequence() const noexcept {
       return t_.commit_sequence();
    }

    constexpr timestamp_type timestamp() const noexcept {
       return t_.timestamp();
    }

    constexpr void clear_changes() {
       t_.clear_changes();
    }

    template<class EventIterator>
    constexpr void put(EventIterator const b, EventIterator const e) {
       t_.put(b, e);
    }

    template<class Event>
    constexpr void put(Event const &evt) {
       this->put(&evt, &evt + 1);
    }
};

}
