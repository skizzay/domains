#include "skizzay/domains/event_sourcing/commit_store.h"
#include "skizzay/domains/event_sourcing/event_stream_provider.h"
#include "skizzay/domains/event_sourcing/hook.h"
#include "skizzay/domains/event_sourcing/optimistic_event_store.h"
#include "skizzay/domains/event_sourcing/traits.h"
#include <catch.hpp>
#include <string>
#include <vector>

using namespace skizzay::domains::event_sourcing;

SCENARIO( "Optimistic Event Sourcing", "[integration][optimistic_event_steam_provider][optimistic_event_stream]" ) {
   using event_type = std::string;
   using commit_type = commit<event_type>;
   using event_collection_type = std::vector<std::string>;

   struct fake_commit_store {
      std::vector<commit_type> commits;
   } commit_store_;

   commit_store cs{std::ref(commit_store_), hook{}};
   event_stream_provider target{optimistic_event_steam_provider<decltype(cs), event_collection_type>{
      std::ref(cs)
   }};
}
