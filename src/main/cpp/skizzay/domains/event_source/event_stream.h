#pragma once

#include <skizzay/domains/event_source/event.h>
#include <skizzay/domains/event_source/commit.h>
#include <functional>
#include <limits>
#include <ranges>

namespace skizzay::domains::event_source {

inline namespace events_details_ {
inline constexpr struct events_function_ final {
   template<concepts::sequenced Sequence>
   static inline constexpr Sequence max_ending_inclusive_sequence{std::numeric_limits<typename Sequence::value_type>::max()};

   template <typename EventStream>
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> starting_exclusive_sequence) const noexcept(
      std::is_nothrow_invocable_v<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>
   ) -> std::invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> {
      return std::invoke(*this, static_cast<EventStream const &>(e), starting_exclusive_sequence, events_function_::max_ending_inclusive_sequence<event_stream_sequence_t<EventStream>>);
   }

   template <typename EventStream>
   requires
      skizzay::domains::tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> &&
      concepts::event_range<skizzay::domains::tag_invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>>
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> starting_exclusive_sequence, event_stream_sequence_t<EventStream> ending_inclusive_sequence) const
      noexcept(skizzay::domains::nothrow_tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>)
         -> skizzay::domains::tag_invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> {
      return skizzay::domains::tag_invoke(*this, static_cast<EventStream const &>(e), starting_exclusive_sequence, ending_inclusive_sequence);
   }

   template <typename EventStream>
   requires (!skizzay::domains::tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>)
      && requires(EventStream const &e, event_stream_sequence_t<EventStream> s) {
         { e.events(s, s.next()) } -> concepts::event_range;
      }
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> starting_exclusive_sequence, event_stream_sequence_t<EventStream> ending_inclusive_sequence) const noexcept(noexcept(e.events(starting_exclusive_sequence, ending_inclusive_sequence)))
      -> decltype(e.events(starting_exclusive_sequence, ending_inclusive_sequence)) {
      return e.events(starting_exclusive_sequence, ending_inclusive_sequence);
   }
} events = {};
} // namespace events_details_

inline namespace put_events_details_ {
inline constexpr struct put_events_function_ final {
   template<typename EventStream, concepts::event_range EventRange>
   requires
      skizzay::domains::tag_invocable<put_events_function_, EventStream &, EventRange &&> &&
      concepts::commit<skizzay::domains::tag_invoke_result_t<put_events_function_, EventStream &, EventRange &&>>
   constexpr auto operator()(EventStream &es, EventRange &&er) const noexcept(
      skizzay::domains::nothrow_tag_invocable<put_events_function_, EventStream &, EventRange &&>
   ) {
      return skizzay::domains::tag_invoke(*this, static_cast<EventStream &>(es), std::forward<EventRange>(er));
   }

   template<typename EventStream, concepts::event_range EventRange>
   requires (!skizzay::domains::tag_invocable<put_events_function_, EventStream &, EventRange const &>)
      && requires(EventStream &es, EventRange &&er) {
         { es.put_events(er) } -> concepts::commit;
      }
   constexpr auto operator()(EventStream &es, EventRange &&er) const noexcept(
      noexcept(es.put_events(std::forward<EventRange>(er)))
   ) -> decltype(
      es.put_events(std::forward<EventRange>(er))
   ) {
      return es.put_events(std::forward<EventRange>(er));
   }
} put_events = {};
}

namespace concepts {
template<typename T>
concept event_stream = event<typename T::event_type>
   && requires (T const &tc) {
      { skizzay::domains::event_source::event_stream_id(tc) } -> identifier;
      { skizzay::domains::event_source::events(tc, std::declval<event_stream_sequence_t<T>>()) } -> event_range;
      { skizzay::domains::event_source::events(tc, std::declval<event_stream_sequence_t<T>>(), std::declval<event_stream_sequence_t<T>>().next()) } -> event_range;
      requires std::same_as<
            event_t<decltype(skizzay::domains::event_source::events(tc, std::declval<event_stream_sequence_t<T>>(), std::declval<event_stream_sequence_t<T>>().next()))>,
            event_t<T>
      >;
};
}

namespace details_ {
template<typename, typename, typename=void>
struct put_events_result_type_impl;

template<concepts::event_stream EventStream, concepts::event_range EventRange>
struct put_events_result_type_impl<EventStream, EventRange, std::void_t<std::invoke_result_t<decltype(skizzay::domains::event_source::put_events), EventStream &, EventRange>>> {
   using type = std::invoke_result_t<decltype(skizzay::domains::event_source::put_events), EventStream &, EventRange>;
};

template<typename, typename>
struct commit_type_impl;

template<concepts::event_stream EventStream, concepts::event_range EventRange>
   requires concepts::commit<typename put_events_result_type_impl<EventStream, EventRange>::type>
struct commit_type_impl<EventStream, EventRange> {
   using type = typename put_events_result_type_impl<EventStream, EventRange>::type;
};
}

template<concepts::event_stream EventStream, concepts::event_range EventRange>
using commit_t = typename details_::commit_type_impl<EventStream, EventRange>::type;

namespace concepts {

template<typename EventStream, typename EventRange>
concept event_range_writer = event_stream<EventStream>
   && event_range<EventRange>
   && std::same_as<event_stream_id_t<EventRange>, event_stream_id_t<EventStream>>
   && std::same_as<event_stream_sequence_t<EventRange>, event_stream_sequence_t<EventStream>>
   && std::same_as<event_stream_timestamp_t<EventRange>, event_stream_timestamp_t<EventStream>>
   && std::same_as<event_stream_id_t<EventRange>, event_stream_id_t<commit_t<EventStream, EventRange>>>
   && std::same_as<event_stream_timestamp_t<EventRange>, decltype(std::declval<commit_t<EventStream, EventRange>>().commit_timestamp())>
   && std::same_as<event_stream_sequence_t<EventRange>, decltype(std::declval<commit_t<EventStream, EventRange>>().event_stream_starting_sequence())>;

}

template<concepts::event_stream EventStream, concepts::sequenced Sequence>
using events_t = decltype(skizzay::domains::event_source::events(std::declval<EventStream>(), std::declval<Sequence>()));

}