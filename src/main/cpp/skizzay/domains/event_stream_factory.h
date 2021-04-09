#pragma once

#include <skizzay/domains/commit.h>
#include <skizzay/domains/event.h>
#include <skizzay/domains/event_stream.h>

namespace skizzay::domains {

inline namespace get_event_stream_details_ {
template<typename EventStream, typename EventStreamId>
concept event_stream_result = concepts::event_stream<EventStream>
   && concepts::identifier<EventStreamId>
   && requires (EventStream const &ce) {
      { event_stream_id(ce) } -> std::same_as<EventStreamId>;
   };

inline constexpr struct get_event_stream_function_ final {

   template <typename EventStreamFactory, typename EventStreamId>
   requires tag_invocable<get_event_stream_function_, EventStreamFactory, EventStreamId>
      && event_stream_result<
            tag_invoke_result_t<get_event_stream_function_, EventStreamFactory, EventStreamId>,
            EventStreamId
         >
   constexpr auto operator()(EventStreamFactory const &ef, EventStreamId &&id) const noexcept(
      nothrow_tag_invocable<get_event_stream_function_, EventStreamFactory, EventStreamId>
   ) -> tag_invoke_result_t<get_event_stream_function_, EventStreamFactory, EventStreamId> {
      return tag_invoke(*this, ef, std::forward<EventStreamId>(id));
   }

   template <typename EventStreamFactory, typename EventStreamId>
   requires (!tag_invocable<get_event_stream_function_, EventStreamFactory, EventStreamId>)
      && std::invocable<EventStreamFactory const &, EventStreamId>
      && event_stream_result<std::invoke_result_t<get_event_stream_function_, EventStreamId>, EventStreamId>
   constexpr auto operator()(EventStreamFactory const &ef, EventStreamId id) const noexcept(
      std::is_nothrow_invocable_v<EventStreamFactory &&, EventStreamId>
   ) -> std::invoke_result_t<EventStreamFactory &&, EventStreamId> {
      return std::invoke(ef, std::forward<EventStreamId>(id));
   }

   template <typename EventStreamFactory, typename EventStreamId>
   requires (!tag_invocable<get_event_stream_function_, EventStreamFactory &&>)
      && requires(EventStreamFactory const &ef, EventStreamId &&id) {
            { ef.get_event_stream(std::forward<EventStreamId>(id)) } -> event_stream_result<EventStreamId>;
         }
   constexpr auto operator()(EventStreamFactory const &ef, EventStreamId id) const noexcept(
      std::declval<EventStreamFactory>().get_event_stream(std::declval<EventStreamId>())
   ) -> decltype(ef.get_event_stream(std::forward<EventStreamId>(id))) {
      return ef.get_event_stream(std::forward<EventStreamId>(id));
   }

   template <typename EventStreamFactory, typename EventStreamId>
   requires (!tag_invocable<get_event_stream_function_, EventStreamFactory &&, EventStreamId>)
      && concepts::dereferenceable<EventStreamFactory>
      && std::invocable<get_event_stream_function_, dereferenced_t<EventStreamFactory>, EventStreamId>
      && event_stream_result<std::invoke_result_t<get_event_stream_function_, dereferenced_t<EventStreamFactory>, EventStreamId>, EventStreamId>
   constexpr auto operator()(EventStreamFactory const &ef, EventStreamId id) const noexcept(
      std::is_nothrow_invocable_v<get_event_stream_function_, dereferenced_t<EventStreamFactory>, EventStreamId>
   ) -> std::invoke_result_t<get_event_stream_function_, dereferenced_t<EventStreamFactory>, EventStreamId> {
      return std::invoke(*this, get_reference(ef), std::forward<EventStreamId>(id));
   }
} get_event_stream = {};

}

inline namespace put_event_stream_details_ {
inline constexpr struct put_event_stream_function_ final {
   template <typename EventStreamStore, typename EventStream, typename EventRange>
   requires tag_invocable<put_event_stream_function_, EventStreamStore, EventStream, EventRange>
      && concepts::commit<
            tag_invoke_result_t<put_event_stream_function_, EventStreamStore, EventStream, EventRange>
         >
   constexpr auto operator()(EventStreamStore &&ess, EventStream &&es, EventRange &&er) const noexcept(
      nothrow_tag_invocable<put_event_stream_function_, EventStreamStore, EventStream, EventRange>
   ) -> tag_invoke_result_t<put_event_stream_function_, EventStreamStore, EventStream, EventRange> {
      return tag_invoke(*this, std::forward<EventStreamStore>(ess), std::forward<EventStream>(es), std::forward<EventRange>(er));
   }

   template <typename EventStreamStore, typename EventStream, typename EventRange>
   requires (!tag_invocable<put_event_stream_function_, EventStreamStore, EventStream, EventRange>)
      && std::invocable<EventStreamStore, EventStream, EventRange>
      && concepts::commit<std::invoke_result_t<EventStreamStore, EventStream, EventRange>>
   constexpr auto operator()(EventStreamStore &&ess, EventStream &&es, EventRange &&er) const noexcept(
      std::is_nothrow_invocable_v<EventStreamStore, EventStream, EventRange>
   ) -> std::invoke_result_t<EventStreamStore, EventStream, EventRange> {
      return std::invoke(std::forward<EventStreamStore>(ess), std::forward<EventStream>(es), std::forward<EventRange>(er));
   }

   template <typename EventStreamStore, typename EventStream, typename EventRange>
   requires (!tag_invocable<put_event_stream_function_, EventStreamStore, EventStream, EventRange>)
      && requires(EventStreamStore &&ess, EventStream &&es, EventRange &&er) {
            { std::forward<EventStreamStore>(ess).put_event_stream(std::forward<EventStream>(es), std::forward<EventRange>(er)) } -> concepts::commit;
         }
   constexpr auto operator()(EventStreamStore &&ess, EventStream &&es, EventRange &&er) const noexcept(noexcept(
      std::declval<EventStreamStore>().put_event_stream(std::declval<EventStream>(), std::declval<EventRange>())
   )) -> decltype(std::forward<EventStreamStore>(ess).put_event_stream(std::forward<EventStream>(es), std::forward<EventRange>(er))) {
      return std::forward<EventStreamStore>(ess).put_event_stream(std::forward<EventStream>(es), std::forward<EventRange>(er));
   }

   template <typename EventStreamStore, typename EventStream, typename EventRange>
   requires (!tag_invocable<put_event_stream_function_, EventStreamStore, EventStream, EventRange>)
      && concepts::dereferenceable<EventStreamStore>
      && std::invocable<get_event_stream_function_, dereferenced_t<EventStreamStore>, EventStream, EventRange>
      && concepts::commit<std::invoke_result_t<get_event_stream_function_, dereferenced_t<EventStreamStore>, EventStream, EventRange>>
   constexpr auto operator()(EventStreamStore &&ess, EventStream &&es, EventRange &&er) const noexcept(
      std::is_nothrow_invocable_v<get_event_stream_function_, dereferenced_t<EventStreamStore>, EventStream, EventRange>
   ) -> std::invoke_result_t<get_event_stream_function_, dereferenced_t<EventStreamStore>, EventStream, EventRange> {
      return std::invoke(*this, get_reference(ess), std::forward<EventStream>(es), std::forward<EventRange>(er));
   }
} put_event_stream = {};

}

namespace concepts {
   template<typename EventStreamFactory, typename EventStreamId>
   concept event_stream_factory = identifier<EventStreamId>
      && requires (EventStreamFactory &&es, EventStreamId &&id) {
            { get_event_stream(std::forward<EventStreamFactory>(es), std::forward<EventStreamId>(id)) } -> event_stream;
            { event_stream_id(std::declval<decltype(get_event_stream(std::forward<EventStreamFactory>(es), std::forward<EventStreamId>(id)))>()) } -> std::same_as<EventStreamId>;
         };

#if 0
   && std::same_as<event_stream_id_t<EventRange>, event_stream_id_t<commit_t<EventStream, EventRange>>>
   && std::same_as<event_stream_timestamp_t<EventRange>, decltype(std::declval<commit_t<EventStream, EventRange>>().commit_timestamp())>
   && std::same_as<event_stream_sequence_t<EventRange>, decltype(std::declval<commit_t<EventStream, EventRange>>().event_stream_starting_sequence())>;
#endif

   template<typename EventStreamStore, typename EventStream, typename EventRange>
   concept event_stream_store = event_stream_factory<EventStreamStore, event_stream_id_t<EventStream>>
      // && event_stream_writer<EventStream, EventRange>
      && requires (EventStreamStore &&ess, EventStream &&es, EventRange &&er) {
            { put_event_stream(std::forward<EventStreamStore>(ess), std::forward<EventStream>(es), std::forward<EventRange>(er)) } -> commit;
         };
} // namespace concepts


template<typename EventStore>
struct event_store_base {
};

} // namespace skizzay::domains