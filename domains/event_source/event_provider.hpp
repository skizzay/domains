#pragma once

namespace domains {
template <class EncodedType, class IdType = uint64_t>
struct event_stream_provider {
   event_stream<EncodedType, IdType>
   get_event_stream(id_type const bucket_id, id_type const stream_id, uint32_t min_version,
                    uint32_t max_version); // Gets an existing event stream
   event_stream<EncodedType, IdType>
   create_event_stream(id_type const bucket_id,
                       id_type const stream_id); // Creates a new event stream
};
}
