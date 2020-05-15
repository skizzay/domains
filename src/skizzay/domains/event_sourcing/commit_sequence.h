#pragma once

#include "skizzay/domains/event_sourcing/sequence.h"
#include <skizzay/utilz/traits.h>

namespace skizzay::domains::event_sourcing {

#if 0
template<class T>
using basic_commit_sequence = basic_sequence<struct commit_sequence_tag, T>;
#elif 1

template<class T>
struct basic_commit_sequence : basic_sequence<utilz::tag<struct commit_sequence_tag>, T> {
    using basic_sequence<utilz::tag<struct commit_sequence_tag>, T>::basic_sequence;
};
#endif


}