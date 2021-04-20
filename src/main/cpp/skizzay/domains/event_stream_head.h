#pragma once

#include <skizzay/domains/concepts.h>
#include <tuple>
#include <type_traits>

namespace skizzay::domains {

namespace concepts {

namespace details_ {

template<typename>
struct is_event_stream_head : std::false_type {
};

template<concepts::identifier EventStreamId, concepts::sequenced EventStreamSequence, concepts::timestamp EventStreamTimestamp>
struct is_event_stream_head<std::tuple<EventStreamId, EventStreamSequence, EventStreamTimestamp>> : std::true_type {
};

}

template<typename T>
concept event_stream_head = details_::is_event_stream_head<T>::value;
}

}