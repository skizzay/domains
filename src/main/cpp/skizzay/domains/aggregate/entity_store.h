#pragma once

#include <skizzay/domains/aggregate/entity.h>
#include <skizzay/domains/event_source/commit.h>
#include <skizzay/domains/event_source/event.h>

namespace skizzay::domains::aggregate {

namespace details_ {
template<typename Entity, typename EntityId>
concept entity_result = concepts::entity<Entity>
   && skizzay::domains::concepts::identifier<EntityId>
   && requires (Entity const &ce) {
      { entity_id(ce) } -> std::same_as<EntityId>;
   };
}

inline namespace get_entity_details_ {
inline constexpr struct get_entity_function_ final {
   template <typename EntityFactory, typename EntityId>
   requires skizzay::domains::tag_invocable<get_entity_function_, EntityFactory &&, EntityId>
      && details_::entity_result<
            skizzay::domains::tag_invoke_result_t<get_entity_function_, EntityFactory &&, EntityId>,
            EntityId
         >
   constexpr auto operator()(EntityFactory &&ef, EntityId &&id) const noexcept(
      skizzay::domains::nothrow_tag_invocable<get_entity_function_, EntityFactory &&, EntityId>
   ) -> skizzay::domains::tag_invoke_result_t<get_entity_function_, EntityFactory &&, EntityId> {
      return skizzay::domains::tag_invoke(*this, std::forward<EntityFactory>(ef), std::forward<EntityId>(id));
   }

   template <typename EntityFactory, typename EntityId>
   requires (!skizzay::domains::tag_invocable<get_entity_function_, EntityFactory &&, EntityId>)
      && std::invocable<EntityFactory &&, EntityId>
      && details_::entity_result<std::invoke_result_t<EntityFactory, EntityId>, EntityId>
   constexpr auto operator()(EntityFactory &&ef, EntityId id) const noexcept(
      std::is_nothrow_invocable_v<EntityFactory &&, EntityId>
   ) -> std::invoke_result_t<EntityFactory &&, EntityId> {
      return std::invoke(std::forward<EntityFactory>(ef), std::forward<EntityId>(id));
   }

   template <typename EntityFactory, typename EntityId>
   requires (!skizzay::domains::tag_invocable<get_entity_function_, EntityFactory &&>)
      && requires(EntityFactory &&ef, EntityId &&id) {
            { std::forward<EntityFactory>(ef).get_entity(std::forward<EntityId>(id)) } -> details_::entity_result<EntityId>;
         }
   constexpr auto operator()(EntityFactory &&ef, EntityId id) const noexcept(noexcept(
      std::forward<EntityFactory>(ef).get_entity(std::forward<EntityId>(id))
   )) -> decltype(std::forward<EntityFactory>(ef).get_entity(std::forward<EntityId>(id))) {
      return std::forward<EntityFactory>(ef).get_entity(std::forward<EntityId>(id));
   }

   template <typename EntityFactory, typename EntityId>
   requires (!skizzay::domains::tag_invocable<get_entity_function_, EntityFactory &&, EntityId>)
      && (!std::invocable<EntityFactory &&, EntityId>)
      && skizzay::domains::concepts::dereferenceable<EntityFactory>
      && std::invocable<get_entity_function_, skizzay::domains::dereferenced_t<EntityFactory>, EntityId>
      && details_::entity_result<std::invoke_result_t<get_entity_function_, skizzay::domains::dereferenced_t<EntityFactory>, EntityId>, EntityId>
   constexpr auto operator()(EntityFactory &&ef, EntityId id) const noexcept(
      std::is_nothrow_invocable_v<get_entity_function_, skizzay::domains::dereferenced_t<EntityFactory>, EntityId>
   ) -> std::invoke_result_t<get_entity_function_, skizzay::domains::dereferenced_t<EntityFactory>, EntityId> {
      return std::invoke(*this, skizzay::domains::get_reference(ef), std::forward<EntityId>(id));
   }
} get_entity = {};

}

inline namespace put_entity_details_ {
inline constexpr struct put_entity_function_ final {
   template <typename EntityStore, typename Entity>
   requires skizzay::domains::tag_invocable<put_entity_function_, EntityStore &&, Entity &&>
      && skizzay::domains::event_source::concepts::commit<
            skizzay::domains::tag_invoke_result_t<put_entity_function_, EntityStore, Entity>
         >
   constexpr auto operator()(EntityStore &&es, Entity &&e) const noexcept(
      skizzay::domains::nothrow_tag_invocable<put_entity_function_, EntityStore, Entity>
   ) -> skizzay::domains::tag_invoke_result_t<put_entity_function_, EntityStore, Entity> {
      return skizzay::domains::tag_invoke(*this, std::forward<EntityStore>(es), std::forward<Entity>(e));
   }

   template <typename EntityStore, typename Entity>
   requires (!skizzay::domains::tag_invocable<put_entity_function_, EntityStore &&, Entity &&>)
      && std::invocable<EntityStore, Entity>
      && skizzay::domains::event_source::concepts::commit<
            std::invoke_result_t<EntityStore, Entity>
         >
   constexpr auto operator()(EntityStore &&es, Entity &&e) const noexcept(
      std::is_nothrow_invocable_v<EntityStore, Entity>
   ) -> std::invoke_result_t<EntityStore, Entity> {
      return std::invoke(std::forward<EntityStore>(es), std::forward<Entity>(e));
   }

   template <typename EntityStore, typename Entity>
   requires (!skizzay::domains::tag_invocable<put_entity_function_, EntityStore &&, Entity &&>)
      && requires(EntityStore &&es, Entity &&e) {
         { es.put_entity(std::forward<Entity>(e)) } -> skizzay::domains::event_source::concepts::commit;
      }
   constexpr auto operator()(EntityStore &&es, Entity &&e) const noexcept(noexcept(
      std::forward<EntityStore>(es).put_entity(std::forward<Entity>(e))
   )) -> decltype(std::forward<EntityStore>(es).put_entity(std::forward<Entity>(e))) {
      return std::forward<EntityStore>(es).put_entity(std::forward<Entity>(e));
   }

   template <typename EntityStore, typename Entity>
   requires (!skizzay::domains::tag_invocable<put_entity_function_, EntityStore &&, Entity &&>)
      && skizzay::domains::concepts::dereferenceable<EntityStore>
      && std::invocable<put_entity_function_, skizzay::domains::dereferenced_t<EntityStore>, Entity>
   constexpr auto operator()(EntityStore &&es, Entity &&e) const noexcept(
      std::is_nothrow_invocable_v<put_entity_function_, skizzay::domains::dereferenced_t<EntityStore>, Entity>
   ) -> std::invoke_result_t<put_entity_function_, skizzay::domains::dereferenced_t<EntityStore>, Entity> {
      return std::invoke(*this, skizzay::domains::get_reference(es), std::forward<Entity>(e));
   }
} put_entity = {};

}

namespace concepts {
template<typename EntityFactory, typename Entity>
concept entity_factory = entity<Entity>
   && requires (EntityFactory factory, entity_id_t<Entity> id) {
      { skizzay::domains::aggregate::get_entity(factory, id) } -> std::same_as<Entity>;
   };

template<typename EntityStore, typename Entity>
concept entity_store = entity_factory<EntityStore, Entity>
   && requires (EntityStore &es, Entity const &e) {
      { skizzay::domains::aggregate::put_entity(es, e) };
   };
}


template<
   concepts::entity Entity,
   typename ...ConstructorArgs
>
struct basic_entity_factory {
   constexpr basic_entity_factory(ConstructorArgs ...constructor_args) :
      constructor_args_{std::move(constructor_args)...}
   {
   }

   constexpr Entity get_entity(entity_id_t<Entity> id) const {
      return std::make_from_tuple<Entity>(
         std::tuple_cat(
            std::forward_as_tuple(std::move(id), entity_version_t<Entity>{}),
            constructor_args_
         )
      );
   }

private:
   [[no_unique_address]] std::tuple<ConstructorArgs...> constructor_args_;
};


template<
   concepts::entity Entity,
   concepts::entity_factory<Entity> EntityFactory,
   typename Alloc=std::pmr::polymorphic_allocator<Entity>
>
struct shared_ptr_entity_factory {
   using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>;
   using result_type = std::shared_ptr<Entity>;

   constexpr shared_ptr_entity_factory(EntityFactory ef, allocator_type alloc={}) noexcept(
      std::is_nothrow_move_constructible_v<EntityFactory>
      && std::is_nothrow_move_constructible_v<allocator_type>
   ) :
      entity_factory_{std::move(ef)},
      allocator_{std::move(alloc)}
   {
   }

   constexpr result_type get_entity([[maybe_unused]] entity_id_t<Entity> id) {
      return std::allocate_shared<Entity>(
         allocator_,
         skizzay::domains::aggregate::get_entity(entity_factory_, std::move(id))
      );
   }

private:
   [[no_unique_address]] EntityFactory entity_factory_;
   [[no_unique_address]] allocator_type allocator_;
};


template<typename Allocator>
struct destroy_and_deallocate {
   [[no_unique_address]] Allocator allocator;

   constexpr void operator()(typename std::allocator_traits<Allocator>::pointer object) {
      if (nullptr != object) {
         if constexpr (!std::is_trivially_destructible_v<typename Allocator::value_type>) {
            std::allocator_traits<Allocator>::destroy(allocator, object);
         }
         std::allocator_traits<Allocator>::deallocate(allocator, object, 1);
      }
   }
};


template<
   concepts::entity Entity,
   concepts::entity_factory<Entity> EntityFactory,
   typename Alloc=std::pmr::polymorphic_allocator<Entity>
>
struct unique_ptr_entity_factory {
   using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<Entity>;
   using result_type = std::unique_ptr<Entity, destroy_and_deallocate<allocator_type>>;

   constexpr unique_ptr_entity_factory(EntityFactory ef, allocator_type alloc={}) noexcept(
      std::is_nothrow_move_constructible_v<EntityFactory>
      && std::is_nothrow_move_constructible_v<allocator_type>
   ) :
      entity_factory_{std::move(ef)},
      allocator_{std::move(alloc)}
   {
   }

   constexpr result_type get_entity(entity_id_t<Entity> id) {
      auto *p = std::allocator_traits<allocator_type>::allocate(allocator_, 1);
      std::allocator_traits<allocator_type>::construct(
         allocator_,
         p,
         skizzay::domains::aggregate::get_entity(entity_factory_, std::move(id))
      );
      return result_type{p, destroy_and_deallocate{allocator_}};
   }

private:
   [[no_unique_address]] EntityFactory entity_factory_;
   [[no_unique_address]] allocator_type allocator_;
};

}