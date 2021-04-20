#pragma once

#include <skizzay/domains/concepts.h>
#include <skizzay/domains/entity.h>
#include <skizzay/domains/event.h>
#include <algorithm>
#include <vector>

namespace skizzay::domains {

inline namespace uncommitted_events_details_ {
inline constexpr struct uncommitted_events_function_ final {
   template <typename AggregateRoot>
   requires tag_invocable<uncommitted_events_function_, AggregateRoot const &>
      && concepts::event_range<
            tag_invoke_result_t<uncommitted_events_function_, AggregateRoot const &>
         >
   constexpr auto operator()(AggregateRoot const &e) const noexcept(
      nothrow_tag_invocable<uncommitted_events_function_, AggregateRoot const &>
   ) -> tag_invoke_result_t<uncommitted_events_function_, AggregateRoot const &> {
      return tag_invoke(*this, static_cast<AggregateRoot const &>(e));
   }

   template <typename AggregateRoot>
   requires requires(AggregateRoot const &e) {
      { e.uncommitted_events() } -> concepts::event_range;
      requires !tag_invocable<uncommitted_events_function_, AggregateRoot const &>;
   }
   constexpr auto operator()(AggregateRoot const &e) const noexcept(noexcept(e.uncommitted_events()))
      -> decltype(e.uncommitted_events()) {
      return e.uncommitted_events();
   }

   template <typename AggregateRoot>
   requires requires(AggregateRoot const &e) {
      { *e };
      requires std::invocable<uncommitted_events_function_, decltype(*e)>;
      requires concepts::event_range<std::invoke_result_t<uncommitted_events_function_, decltype(*e)>>;
      requires !tag_invocable<uncommitted_events_function_, AggregateRoot const &>;
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
   requires tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>
      && std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
   constexpr void operator()(AggregateRoot &ar) const noexcept(
      nothrow_tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>
   ) {
      tag_invoke(*this, static_cast<AggregateRoot &>(ar));
   }

   template <typename AggregateRoot>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && (!tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>)
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
      && (!tag_invocable<clear_uncommitted_events_function_, AggregateRoot &>)
      && concepts::dereferenceable<AggregateRoot>
      && std::invocable<clear_uncommitted_events_function_, dereferenced_t<AggregateRoot>>
   constexpr void operator()(AggregateRoot &ar) const noexcept(
      std::is_nothrow_invocable_v<clear_uncommitted_events_function_, dereferenced_t<AggregateRoot>>
   ) {
      std::invoke(*this, *ar);
   }
} clear_uncommitted_events = {};

}


inline namespace load_aggregate_root_from_history_details_ {
inline constexpr struct load_aggregate_root_from_history_function_ final {
   template<typename AggregateRoot>
   using uncommitted_event_t = event_t<
      decltype(uncommitted_events(std::declval<AggregateRoot &>()))
   >;

   template <typename AggregateRoot, typename EventRange>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && concepts::entity<AggregateRoot>
      && concepts::event_range<EventRange>
      && std::same_as<uncommitted_event_t<AggregateRoot>, event_t<EventRange>>
      && tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange const &>
   constexpr void operator()(AggregateRoot &ar, EventRange const &er) const noexcept(
      nothrow_tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange const &>
   ) {
      tag_invoke(*this, static_cast<AggregateRoot &>(ar), static_cast<EventRange const &>(er));
   }

   template <typename AggregateRoot, typename EventRange>
   requires std::same_as<AggregateRoot, std::remove_const_t<AggregateRoot>>
      && concepts::entity<AggregateRoot>
      && concepts::event_range<EventRange>
      && std::same_as<uncommitted_event_t<AggregateRoot>, event_t<EventRange>>
      && (!tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange>)
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
      && concepts::event_range<EventRange>
      && std::same_as<uncommitted_event_t<AggregateRoot>, event_t<EventRange>>
      && (!tag_invocable<load_aggregate_root_from_history_function_, AggregateRoot &, EventRange>)
      && concepts::dereferenceable<AggregateRoot>
      && std::invocable<load_aggregate_root_from_history_function_, dereferenced_t<AggregateRoot>, EventRange const &>
   constexpr void operator()(AggregateRoot &ar, EventRange const &er) const noexcept(
      std::is_nothrow_invocable_v<load_aggregate_root_from_history_function_, dereferenced_t<AggregateRoot>, EventRange const &>
   ) {
      std::invoke(*this, *ar, er);
   }
} load_aggregate_root_from_history = {};

}

namespace concepts {

template<typename AggregateRoot>
concept aggregate_root = entity<AggregateRoot>
   && requires (AggregateRoot const &car, AggregateRoot &ar) {
      { clear_uncommitted_events(ar) };
      { uncommitted_events(car) } -> concepts::event_range;
   };

template<typename EventRange, typename AggregateRoot>
concept history_for_aggregate_root = aggregate_root<AggregateRoot>
   && concepts::event_range<EventRange>
   && requires(AggregateRoot &ar, EventRange const &er) {
      { load_aggregate_root_from_history(ar, er) };
   };
}

#define DECLARE_DOMAINS_AGGREGATE_ROOT(...) \
   friend skizzay::domains::aggregate_root_base<__VA_ARGS__>; \
   using skizzay::domains::aggregate_root_base<__VA_ARGS__>::on


template<
   typename Entity,
   concepts::event Event,
   typename UncommittedEvents=std::pmr::vector<Event>
>
class aggregate_root_base {
public:
   using event_type = Event;
   using entity_id_type = event_stream_id_t<Event>;
   using entity_version_type = event_stream_sequence_t<Event>;

   template<concepts::event_range_of<Event> EventRange>
   constexpr void load_from_history(EventRange const &events) {
      std::ranges::for_each(events, [this](Event const &event) mutable {
         handle_event(event);
         current_committed_version_ = event_stream_sequence(event);
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

   constexpr aggregate_root_base(entity_id_type entity_id, entity_version_type entity_version, allocator_type alloc={}) noexcept(
      std::is_nothrow_move_constructible_v<entity_id_type> && std::is_nothrow_move_constructible_v<entity_version_type>
   ) :
      entity_id_{std::move(entity_id)},
      current_committed_version_{std::move(entity_version)},
      uncommitted_events_{std::move(alloc)}
   {
   }

   constexpr aggregate_root_base(entity_id_type entity_id, allocator_type alloc={}) noexcept(
      std::is_nothrow_move_constructible_v<entity_id_type> && std::is_nothrow_default_constructible_v<entity_version_type>
   ) :
      aggregate_root_base{std::move(entity_id), {}, std::move(alloc)}
   {
   }

   template<concepts::event E, typename ...Args>
      requires std::is_convertible_v<E, Event>
   constexpr E make_event(Args &&...args) const {
      return E{entity_id(), entity_version().next(), std::forward<Args>(args)...};
   }

   template<concepts::event E>
      requires std::is_constructible_v<Event, std::add_rvalue_reference_t<E>>
   constexpr void buffer_event(E &&event) {
      uncommitted_events_.emplace_back(std::forward<E>(event));
   }

   template<concepts::event E, typename ...Args>
      requires std::is_constructible_v<Event, std::add_rvalue_reference_t<E>>
   constexpr void apply_event(Args &&...args) {
      E event = this->make_event<E>(std::forward<Args>(args)...);
      handle_event(event);
      buffer_event(std::move(event));
   }

   template<concepts::event E>
      requires (!std::same_as<E, Event>)
   constexpr void handle_event(E const &e) {
      static_cast<Entity *>(this)->on(get_reference(e));
   }

   constexpr void handle_event(Event const &event) {
      dispatch_event([this]<concepts::event E>(E const &e) mutable {
         if constexpr (concepts::same_reference_removed<E, Event>) {
            static_cast<Entity *>(this)->on(get_reference(e));
         }
         else {
            handle_event(get_reference(e));
         }
      }, event);
   }

   template<concepts::event E>
   constexpr void on(E const &) const noexcept {
      // No-op
      // Placeholder for when Entity does not care about a particular event
   }

private:
   entity_id_type entity_id_;
   entity_version_type current_committed_version_;
   UncommittedEvents uncommitted_events_;
};


}