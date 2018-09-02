#pragma once

#include "skizzay/domains/event_sourcing/traits.h"
#include <cstdint>
#include <limits>
#include <utility>

namespace skizzay::domains::event_sourcing {

template<class T>
class event_stream_provider {
   T t_;

public:
   using stream_id_type = typename strip_reference_wrapper_t<T>::stream_id_type;

   constexpr event_stream_provider(T &&t) noexcept :
      t_{std::forward<T>(t)}
   {
   }

   constexpr decltype(auto) create_stream(stream_id_type const &sid) {
      return strip(t_).create_stream(sid);
   }

   constexpr decltype(auto) open_stream(stream_id_type const &sid,
         std::uint32_t const min_sequence=0,
         std::uint32_t const max_sequence=std::numeric_limits<std::uint32_t>::max()) {
      return strip(t_).open_stream(sid, min_sequence, max_sequence);
   }
};

}
