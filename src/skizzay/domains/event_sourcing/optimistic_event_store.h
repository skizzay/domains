#pragma once

#include "skizzay/domains/event_sourcing/commit_store.h"
#include "skizzay/domains/event_sourcing/event_stream.h"
#include "skizzay/domains/event_sourcing/optimistic_event_stream.h"
#include <cstdint>

namespace skizzay::domains::event_sourcing {

template<class CommitStoreType, class EventCollectionType>
class optimistic_event_steam_provider {
   using self_type = optimistic_event_steam_provider<CommitStoreType, EventCollectionType>;
   using event_stream_impl = optimistic_event_stream<self_type, EventCollectionType>;

   CommitStoreType commits_;

public:
   // TODO: Support different allocators
   using stream_id_type = typename CommitStoreType::stream_id_type;

   constexpr optimistic_event_steam_provider(CommitStoreType &&cs) :
      commits_{std::forward<CommitStoreType>(cs)}
   {
   }

   constexpr auto create_stream(stream_id_type const &sid) {
      return event_stream{
         event_stream_impl{
            sid, commits_
         }
      };
   }

   constexpr auto open_stream(stream_id_type const &sid,
         std::uint32_t const min_sequence, std::uint32_t const max_sequence) {
      return event_stream{
         event_stream_impl{
            sid, commits_, min_sequence, max_sequence
         }
      };
   }
};

}
