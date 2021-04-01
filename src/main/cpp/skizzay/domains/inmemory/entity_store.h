#pragma once

#include <skizzay/domains/aggregate/entity.h>
#include <skizzay/domains/aggregate/entity_store.h>

namespace skizzay::domains::aggregate::inmemory {

template<
   skizzay::domains::aggregate::concepts::entity Entity,
   skizzay::domains::aggregate::concepts::entity_factory<Entity> EntityFactory,
   typename Mutex=std::mutex
>
class entity_store {
   using key_type = skizzay::domains::aggregate::entity_id_t<Entity>;
   using value_type = Entity;
   using entity_table = std::pmr::unordered_map<key_type, value_type>;
   using entity_factory_type = EntityFactory;

   [[no_unique_address]] mutable Mutex mutex_;
   entity_table entities_;
   [[no_unique_address]] entity_factory_type entity_factory_;

public:
   constexpr explicit entity_store(entity_factory_type entity_factory={}, entity_table::allocator_type alloc={}) :
      entities_{std::move(alloc)},
      entity_factory_{std::move(entity_factory)}
   {
   }

   constexpr value_type get_entity(key_type id) {
      std::scoped_lock lock{mutex_};
      auto const i = entities_.find(id);
      if (i == std::end(entities_)) {
         return skizzay::domains::aggregate::get_entity(entity_factory_, std::move(id));
      }
      else {
         return i->second;
      }
   }

   constexpr bool put_entity(Entity &&entity) {
      std::scoped_lock lock{mutex_};
      auto [i, created] = entities_.try_emplace(skizzay::domains::aggregate::entity_id(entity), std::forward<Entity>(entity));
      if (!created) {
         i->second = std::forward<Entity>(entity);
      }
      return created;
   }
};

}