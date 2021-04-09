#pragma once

#include <skizzay/domains/aggregate/aggregate_root.h>
#include <skizzay/domains/aggregate/aggregate_root_factory.h>
#include <skizzay/domains/event_source/commit.h>
#include <skizzay/domains/event_source/event_store.h>
#include <iterator>

namespace skizzay::domains::aggregate {

inline constexpr auto ignore_commit = [](skizzay::domains::event_source::concepts::commit auto const &) noexcept {};


template<
   concepts::aggregate_root AggregateRoot,
   concepts::aggregate_root_store<AggregateRoot> AggregateRootStore,
   typename CommandDispatcher,
   typename OnCommitCallback
>
class command_handler {
public:
   constexpr explicit command_handler(AggregateRootStore entity_store, CommandDispatcher command_dispatcher, OnCommitCallback on_commit={}) noexcept :
      entity_store_{std::move(entity_store)},
      command_dispatcher_{std::move(command_dispatcher)},
      on_commit_{std::move(on_commit)}
   {
   }

   template<typename Command>
   constexpr void operator()(Command const &command) noexcept {
      decltype(auto) aggegrate_root{get_aggregate_root(aggregate_root_factory_, entity_id(command))};
      std::invoke(command_dispatcher_, skizzay::domains::get_reference(aggegrate_root), command);
      on_commit_(put_entity(entity_store_, std::move(aggegrate_root)));
   }

private:
   AggregateRootStore entity_store_;
   CommandDispatcher command_dispatcher_;
   OnCommitCallback on_commit_;
};

template<typename AggregateRootStore, typename CommandDispatcher>
command_handler(AggregateRootStore, CommandDispatcher) -> command_handler<AggregateRootStore, CommandDispatcher, decltype(ignore_commit)>;

template<typename AggregateRootStore, typename CommandDispatcher, typename OnCommitCallback>
command_handler(AggregateRootStore, CommandDispatcher, OnCommitCallback) -> command_handler<AggregateRootStore, CommandDispatcher, OnCommitCallback>;
}