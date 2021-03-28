#pragma once

#include <skizzay/domains/aggregate/entity.h>
#include <skizzay/domains/event_source/event.h>
#include <algorithm>
#include <vector>

namespace skizzay::domains::aggregate {

inline namespace uncommitted_events_details_ {
inline constexpr struct uncommitted_events_function_ final {
   template <typename AggregateRoot>
   requires skizzay::domains::tag_invocable<uncommitted_events_function_, AggregateRoot const &>
      && skizzay::domains::event_source::concepts::event_range<
            skizzay::domains::tag_invoke_result_t<uncommitted_events_function_, AggregateRoot const &>
         >
   constexpr auto operator()(AggregateRoot const &e) const noexcept(
      skizzay::domains::nothrow_tag_invocable<uncommitted_events_function_, AggregateRoot const &>
   ) -> skizzay::domains::tag_invoke_result_t<uncommitted_events_function_, AggregateRoot const &> {
      return skizzay::domains::tag_invoke(*this, static_cast<AggregateRoot const &>(e));
   }

   template <typename AggregateRoot>
   requires requires(AggregateRoot const &e) {
      { e.uncommitted_events() } -> skizzay::domains::event_source::concepts::event_range;
      requires !skizzay::domains::tag_invocable<uncommitted_events_function_, AggregateRoot const &>;
   }
   constexpr auto operator()(AggregateRoot const &e) const noexcept(noexcept(e.uncommitted_events()))
      -> decltype(e.uncommitted_events()) {
      return e.uncommitted_events();
   }

   template <typename AggregateRoot>
   requires requires(AggregateRoot const &e) {
      { *e };
      requires std::invocable<uncommitted_events_function_, decltype(*e)>;
      requires skizzay::domains::event_source::concepts::event_range<std::invoke_result_t<uncommitted_events_function_, decltype(*e)>>;
      requires !skizzay::domains::tag_invocable<uncommitted_events_function_, AggregateRoot const &>;
   }
   constexpr auto operator()(AggregateRoot const &e) const
      noexcept(std::is_nothrow_invocable_v<uncommitted_events_function_, decltype(*e)>)
         -> std::invoke_result_t<uncommitted_events_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }
} uncommitted_events = {};

}

inline namespace clear_uncommitted_events_details_ {
inline constexpr struct clear_uncommitted_events_function_ final {
   template <typename AggregateRoot>
   requires skizzay::domains::tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>
      && std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
   constexpr void operator()(AggregateRoot &ar) const noexcept(
      skizzay::domains::nothrow_tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>
   ) {
      skizzay::domains::tag_invoke(*this, static_cast<AggregateRoot &>(ar));
   }

   template <typename AggregateRoot>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && (!skizzay::domains::tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>)
      && requires(AggregateRoot &ar) {
         { ar.clear_uncommitted_events() };
      }
   constexpr void operator()(AggregateRoot &ar) const noexcept(
      noexcept(ar.clear_uncommitted_events())
   ) {
      ar.clear_uncommitted_events();
   }

   template <typename AggregateRoot>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && (!skizzay::domains::tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>)
      && skizzay::domains::concepts::dereferenceable<AggregateRoot>
      && std::invocable<clear_uncommitted_events_function_, skizzay::domains::dereferenced_t<AggregateRoot>>
   constexpr void operator()(AggregateRoot &ar) const noexcept(
      std::is_nothrow_invocable_v<clear_uncommitted_events_function_, skizzay::domains::dereferenced_t<AggregateRoot>>
   ) {
      std::invoke(*this, *ar);
   }
} clear_uncommitted_events = {};

}


inline namespace load_aggregate_root_from_history_details_ {
inline constexpr struct load_aggregate_root_from_history_function_ final {
   template<typename AggregateRoot>
   using uncommitted_event_t = skizzay::domains::event_source::event_t<
      decltype(skizzay::domains::aggregate::uncommitted_events(std::declval<AggregateRoot &>()))
   >;

   template <typename AggregateRoot, typename EventRange>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && concepts::entity<AggregateRoot>
      && skizzay::domains::event_source::concepts::event_range<EventRange>
      && std::same_as<uncommitted_event_t<AggregateRoot>, skizzay::domains::event_source::event_t<EventRange>>
      && skizzay::domains::tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange const &>
   constexpr void operator()(AggregateRoot &ar, EventRange const &er) const noexcept(
      skizzay::domains::nothrow_tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange const &>
   ) {
      skizzay::domains::tag_invoke(*this, static_cast<AggregateRoot &>(ar), static_cast<EventRange const &>(er));
   }

   template <typename AggregateRoot, typename EventRange>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && concepts::entity<AggregateRoot>
      && skizzay::domains::event_source::concepts::event_range<EventRange>
      && std::same_as<uncommitted_event_t<AggregateRoot>, skizzay::domains::event_source::event_t<EventRange>>
      && (!skizzay::domains::tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange>)
      && requires(AggregateRoot &ar, EventRange const &er) {
         { ar.load_from_history(er) };
      }
   constexpr void operator()(AggregateRoot &ar, EventRange const &er) const noexcept(
      noexcept(ar.load_from_history(er))
   ) {
      ar.load_from_history(er);
   }

   template <typename AggregateRoot, typename EventRange>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && concepts::entity<AggregateRoot>
      && skizzay::domains::event_source::concepts::event_range<EventRange>
      && std::same_as<uncommitted_event_t<AggregateRoot>, skizzay::domains::event_source::event_t<EventRange>>
      && (!skizzay::domains::tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange>)
      && skizzay::domains::concepts::dereferenceable<AggregateRoot>
      && std::invocable<load_aggregate_root_from_history_function_, skizzay::domains::dereferenced_t<AggregateRoot>, EventRange const &>
   constexpr void operator()(AggregateRoot &ar, EventRange const &er) const noexcept(
      std::is_nothrow_invocable_v<load_aggregate_root_from_history_function_, skizzay::domains::dereferenced_t<AggregateRoot>, EventRange const &>
   ) {
      std::invoke(*this, *ar, er);
   }
} load_aggregate_root_from_history = {};

}


template<
   typename Entity,
   skizzay::domains::event_source::concepts::event Event,
   typename UncommittedEvents=std::pmr::vector<Event>
>
class aggregate_root {
public:
   using event_type = Event;
   using entity_id_type = skizzay::domains::event_source::event_stream_id_t<Event>;
   using entity_version_type = skizzay::domains::event_source::event_stream_sequence_t<Event>;

   template<skizzay::domains::event_source::concepts::event_range EventRange>
      requires skizzay::domains::event_source::concepts::event_range_of<EventRange, Event>
   constexpr void load_from_history(EventRange const &events) {
      std::ranges::for_each(events, [this](Event const &event) mutable {
         skizzay::domains::event_source::dispatch_event([this](auto const &concrete_event) mutable {
            // static_cast<Entity *>(this)->on(concrete_event);
            current_committed_version_ = event_source::event_stream_sequence(concrete_event);
         }, event);
      });
   }

   constexpr UncommittedEvents const & uncommitted_events() const noexcept {
      return uncommitted_events_;
   }

   constexpr void clear_uncommitted_events() noexcept {
      uncommitted_events_.clear();
   }

   constexpr entity_id_type entity_id() const noexcept {
      return entity_id_;
   }

   constexpr entity_version_type entity_version() const noexcept {
      return entity_version_type{
         current_committed_version_.value()
         + static_cast<typename entity_version_type::value_type>(std::size(uncommitted_events_))
      };
   }

protected:
   using allocator_type = typename UncommittedEvents::allocator_type;

   constexpr aggregate_root(entity_id_type entity_id, entity_version_type entity_version, allocator_type alloc={}) noexcept(
      std::is_nothrow_move_constructible_v<entity_id_type> && std::is_nothrow_move_constructible_v<entity_version_type>
   ) :
      entity_id_{std::move(entity_id)},
      current_committed_version_{std::move(entity_version)},
      uncommitted_events_{std::move(alloc)}
   {
   }

   constexpr aggregate_root(entity_id_type entity_id, allocator_type alloc={}) noexcept(
      std::is_nothrow_move_constructible_v<entity_id_type> && std::is_nothrow_default_constructible_v<entity_version_type>
   ) :
      aggregate_root{std::move(entity_id), {}, std::move(alloc)}
   {
   }

   template<skizzay::domains::event_source::concepts::event AggregateRoot, typename ...Args>
      requires std::is_convertible_v<AggregateRoot, Event>
   constexpr AggregateRoot make_event(Args &&...args) const {
      return AggregateRoot{entity_id(), entity_version().next(), std::forward<Args>(args)...};
   }

   template<skizzay::domains::event_source::concepts::event AggregateRoot>
      requires std::is_constructible_v<Event, std::add_rvalue_reference_t<AggregateRoot>>
   constexpr void buffer_event(AggregateRoot &&event) {
      uncommitted_events_.emplace_back(std::forward<AggregateRoot>(event));
   }

   template<skizzay::domains::event_source::concepts::event AggregateRoot, typename ...Args>
      requires std::is_constructible_v<Event, std::add_rvalue_reference_t<AggregateRoot>>
   constexpr void apply_event(Args &&...args) {
      AggregateRoot event = this->make_event<AggregateRoot>(std::forward<Args>(args)...);
      static_cast<Entity *>(this)->on(const_cast<AggregateRoot const &>(event));
      buffer_event(std::move(event));
   }

private:
   template<skizzay::domains::event_source::concepts::event AggregateRoot>
   constexpr void on(AggregateRoot const &) const noexcept {
      // No-op
      // Placeholder for when Entity does not care about a particular event
   }

   entity_id_type entity_id_;
   entity_version_type current_committed_version_;
   UncommittedEvents uncommitted_events_;
};


}