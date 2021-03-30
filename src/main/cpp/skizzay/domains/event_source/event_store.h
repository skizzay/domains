#pragma once

#include <skizzay/domains/event_source/commit.h>
#include <skizzay/domains/event_source/event_stream.h>

namespace skizzay::domains::event_source {

namespace concepts {
   template<typename EventStore, typename EventStreamId>
   concept event_store = identifier<EventStreamId> &&
      requires (EventStore &es, EventStreamId id) {
         { es.get_event_stream(id) } -> event_stream;
         { event_stream_id_t<decltype(es.get_event_stream(id))> } -> std::same_as<EventStreamId>
      };
} // namespace concepts

} // namespace skizzay::domains::event_source