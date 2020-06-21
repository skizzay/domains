#pragma once

#include "skizzay/domains/event_sourcing/event_header.h"
#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>
#include <ranges>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<concepts::event_header EventHeader, class ...TagsAndEventData>
using basic_event = utilz::dto<
        utilz::tag<struct basic_event_tag>,
        EventHeader,
        TagsAndEventData...>;

namespace concepts {
        template<class T>
        concept event = utilz::is_template_v<T, utilz::dto> && requires {
                0 < utilz::length_v<typename T::tag_types>;
                std::is_same_v<utilz::at_t<typename T::tag_types, 0>, utilz::tag<skizzay::domains::event_sourcing::basic_event_tag>>;
                0 < utilz::length_v<typename T::data_types>;
                requires event_header<utilz::at_t<typename T::data_types, 0>>;
        };

        template<class T>
        concept event_range = std::ranges::range<T> && requires {
                typename T::value_type;
                requires event<typename T::value_type>;
        };
}

}