#pragma once

#include "skizzay/domains/event_sourcing/snapshot_header.h"
#include <skizzay/utilz/dto.h>
#include <skizzay/utilz/traits.h>
#include <type_traits>

namespace skizzay::domains::event_sourcing {

template<class SnapshotHeader, class MementoType>
using basic_snapshot = utilz::dto<
        utilz::tag<struct basic_snapshot_tag>,
        std::enable_if_t<utilz::is_template_v<SnapshotHeader, basic_snapshot_header>, SnapshotHeader>,
        MementoType>;

}