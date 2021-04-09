#pragma once

#include <skizzay/domains/aggregate/aggregate_root.h>
#include <skizzay/domains/aggregate/entity_stream_factory.h>
#include <skizzay/domains/aggregate/entity_store.h>
#include <skizzay/domains/event_source/commit.h>
#include <memory>
#include <memory_resource>
#include <tuple>

namespace skizzay::domains::aggregate {

namespace concepts {
template<typename AggregateRootStore, typename AggregateRoot>
concept aggregate_root_store_for = aggregate_root<AggregateRoot>
   && entity_store<AggregateRootStore, AggregateRoot>;
}

template<
   concepts::aggregate_root AggregateRoot,
   concepts::entity_factory<AggregateRoot> AggregateRootFactory,
   skizzay::domains::event_source::concepts::event_stream_store EventStreamStore
>
   requires std::same_as<entity_id_t<AggregateRoot>, skizzay::domains::event_source::event_stream_id_t<EventStreamStore>>
struct event_sourced_aggregate_root_store {
   constexpr event_sourced_aggregate_root_store(
      AggregateRootFactory aggregate_root_factory,
      EventStreamStore event_stream_factory
   ) :
      aggregate_root_factory_{std::move(aggregate_root_factory)},
      event_stream_factory_{std::move(event_stream_factory)}
   {
   }

   AggregateRoot get_entity(entity_id_t<AggregateRoot> id) {
      decltype(auto) result{get_entity(entity_store_, id)};
      load_aggregate_root_from_history(
         result,
         skizzay::domains::event_source::events(
            get_event_stream(event_stream_factory_, id),
            entity_version(aggregate_root),
            skizzay::domains::event_source::max_ending_exclusive_version<entity_version_t<AggregateRoot>>
         )
      );
      return result;
   }

   auto put_entity(std::add_lvalue_reference_t<AggregateRoot> ar) {
      auto result{put_events(
         get_event_stream(event_stream_factory_, entity_id(ar)),
         uncommitted_events(ar)
      )};
      clear_uncommitted_events(ar);
      return result;
   }

private:
   [[no_unique_address]] AggregateRootFactory aggregate_root_factory_;
   EventStreamStore event_stream_factory_;
};

}