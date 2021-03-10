#pragma once

#include <skizzay/domains/event_source/event.h>
#include <skizzay/domains/event_source/commit.h>
#include <limits>
#include <ranges>

namespace skizzay::domains::event_source {

inline namespace events_details_ {
inline constexpr struct events_function_ final {
   template<concepts::sequenced Sequence>
   inline constexpr Sequence const max_ending_inclusive_sequence = std::numeric_limits<decltype(std::declval<Sequence>().value())>::max();

   template <typename EventStream>
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> const starting_exclusive_sequence) const noexcept(
      std::nothrow_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream> const, event_stream_sequence_t<EventStream> const>
   ) {
      return std::invoke(*this, static_cast<EventStream const &>(e), starting_exclusive_sequence, max_ending_inclusive_sequence<event_stream_sequence_t<EventStream>>);
   }

   template <typename EventStream>
   requires
      skizzay::domains::tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream> const, event_stream_sequence_t<EventStream> const> &&
      concepts::event_range<skizzay::domains::tag_invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>>
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> const starting_exclusive_sequence, event_stream_sequence_t<EventStream> const ending_inclusive_sequence) const
      noexcept(skizzay::domains::nothrow_tag_invocable<events_function_, EventStream const &, Sequence, Sequence>)
         -> skizzay::domains::tag_invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> {
      return skizzay::domains::tag_invoke(*this, static_cast<EventStream const &>(e), starting_exclusive_sequence, ending_inclusive_sequence);
   }

   template <typename EventStream>
   requires requires(EventStream const &e, event_stream_sequence_t<EventStream> const s) {
      { e.events(s, s.next()) } -> concepts::event_range;
      requires !skizzay::domains::tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream> const, event_stream_sequence_t<EventStream> const>;
   }
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> const starting_exclusive_sequence, event_stream_sequence_t<EventStream> const ending_inclusive_sequence) const noexcept(noexcept(e.events(starting_exclusive_sequence, ending_inclusive_sequence)))
      -> decltype(e.events(starting_exclusive_sequence, ending_inclusive_sequence)) {
      return e.events(starting_exclusive_sequence, ending_inclusive_sequence);
   }
} events = {};
} // namespace events_details_

inline namespace put_events_details_ {
inline constexpr struct put_events_function_ final {
   template<typename EventStream, concepts::event_range EventRange>
   requires
      skizzay::domains::tag_invocable<put_events_function_, EventStream &, EventRange const &> &&
      concepts::commit<skizzay::domains::tag_invoke_result_t<put_events_function_, EventStream &, EventRange const &>>
   constexpr auto operator()(EventStream &es, EventRange const &er) const noexcept(
      skizzay::domains::nothrow_tag_invocable<put_events_function_, EventStream &, EventRange const &>
   ) {
      return skizzay::domains::tag_invoke(*this, static_cast<EventStream &>(es), static_cast<EventRange const &>(er));
   }

   template<typename EventStream, concepts::event_range EventRange>
   requires
      !skizzay::domains::tag_invocable<put_events_function_, EventStream &, EventRange const &> &&
      requires(EventStream &es, EventRange const &er) {
         { es.put_events(er) } -> concepts::commit;
      }
   constexpr auto operator()(EventStream &es, EventRange const &er) const noexcept(
      noexcept(es.put_events(er))
   ) -> decltype(
      es.put_events(er)
   ) {
      return es.put_events(er);
   }
} put_events = {};
}

namespace concepts {
template<typename T>
concept event_stream = requires (T const &tc) {
   { typename T::event_type } -> event;
   { skizzay::domains::event_source::event_stream_id(tc) } -> identifier;
   { skizzay::domains::event_source::events(tc, std::declval<event_stream_sequence_t<T>>()) } -> event_range;
   { skizzay::domains::event_source::events(tc, std::declval<event_stream_sequence_t<T>>()), std::declval<event_stream_sequence_t<T>>()).next()) } -> event_range;
};
}

template<concepts::event_stream EventStream, concepts::sequenced Sequence>
using events_t = decltype(skizzay::domains::event_source::events(std::declval<EventStream>(), std::declval<Sequence>()));

}