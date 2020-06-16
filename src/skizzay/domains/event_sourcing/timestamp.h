#pragma once

#include "skizzay/domains/event_sourcing/concepts/timestamp.h"

namespace skizzay::domains::event_sourcing {

template<class T>
        requires concepts::timestamp<T>
inline constexpr T timestamp_not_set = T{};

}