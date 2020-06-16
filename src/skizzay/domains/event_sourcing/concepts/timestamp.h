#pragma once

#include <skizzay/utilz/traits.h>

namespace skizzay::domains::event_sourcing::concepts {

template<class T>
concept timestamp = skizzay::utilz::is_time_point_v<T>;

}