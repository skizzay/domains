#pragma once

#include <domains/utils/concepts.hpp>

namespace domains {

template <typename T>
concept bool Entity = as_concept<traits::entity, T>;

}
