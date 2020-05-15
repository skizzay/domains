#pragma once

#include "skizzay/domains/event_sourcing/sequence.h"
#include <limits>

namespace skizzay::domains::event_sourcing {

template<class T>
using basic_stream_version = basic_sequence<struct stream_version_tag, T>;

}