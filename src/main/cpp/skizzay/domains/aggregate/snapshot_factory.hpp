#pragma once

#include <unordered_map>

namespace domains {
template <class Aggregate>
class snapshot_factory {
public:
   using id_type = typename Aggregate::id_type;
};
}
