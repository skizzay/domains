#pragma once

#include "skizzay/domains/event_sourcing/event_header.h"
#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<concepts::event_header EventHeader, class ...TagsAndEventData>
using basic_event = utilz::dto<
        utilz::tag<struct basic_event_tag>,
        EventHeader,
        TagsAndEventData...>;

namespace concepts {
        template<class T>
        concept event = utilz::is_template_v<T, basic_event>;
}

}