#pragma once

#include <chrono>
#include <type_traits>

namespace domains {
template <class IdType = uint64_t>
struct commit_attempt {
   using event_type = std::decay_t<EventType>;
   using id_type = std::decay_t<IdType>;

   id_type id() const noexcept;
   uint64_t sequence() const noexcept;
   std::chrono::nanoseconds timestamp() const noexcept;
   id_type stream_id() const noexcept;
   uint32_t stream_version() const noexcept;
   auto events() const noexcept; // range of events
};
}
