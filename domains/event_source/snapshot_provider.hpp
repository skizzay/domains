#pragma once

#include <system_error>

namespace domains {
template <class Context, class IdType = uint64_t>
struct snapshot_provider {
   using id_type = std::decay_t<IdType>;
   Context get(id_type const bucket_id, id_type const stream_id, uint32_t max_version);
   std::error_code put(Context const &context);
};
}
