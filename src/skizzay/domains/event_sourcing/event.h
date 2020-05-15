#pragma once

#include "skizzay/domains/event_sourcing/event_header.h"
#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<class EventHeaderType, class ...TagsAndEventData>
using basic_event = utilz::dto<
        utilz::tag<struct basic_event_tag>,
        std::enable_if_t<utilz::is_template_v<EventHeaderType, event_header>, EventHeaderType>,
        TagsAndEventData...>;

}