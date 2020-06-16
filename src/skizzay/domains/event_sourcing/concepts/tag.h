#pragma once

#include <skizzay/utilz/traits.h>

namespace skizzay::domains::event_sourcing::concepts {
    template<class T>
    concept tag = utilz::is_tag_v<T>;
}