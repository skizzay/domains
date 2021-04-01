#pragma once

#include <skizzay/domains/commit.h>
#include <skizzay/domains/event_stream.h>

namespace skizzay::domains {

inline namespace get_event_stream_details_ {
inline constexpr struct get_event_stream_function_ final {
   template<typename EventStream, typename EventStreamId>
   concept event_stream_result = concepts::event_stream<EventStream>
      && skizzay::domains::concepts::identifier<EventStreamId>
      && requires (EventStream const &ce) {
         { event_stream_id(ce) } -> std::same_as<EventStreamId>;
      };

   template <typename EventStreamFactory, typename EventStreamId>
   requires skizzay::domains::tag_invocable<get_event_stream_function_, EventStreamFactory, EventStreamId>
      && event_stream_result<
            skizzay::domains::tag_invoke_result_t<get_event_stream_function_, EventStreamFactory, EventStreamId>,
            EventStreamId
         >
   constexpr auto operator()(EventStreamFactory &&ef, EventStreamId &&id) const noexcept(
      skizzay::domains::nothrow_tag_invocable<get_event_stream_function_, EventStreamFactory &&, EventStreamId>
   ) -> skizzay::domains::tag_invoke_result_t<get_event_stream_function_, EventStreamFactory &&, EventStreamId> {
      return skizzay::domains::tag_invoke(*this, std::forward<EventStreamFactory>(ef), std::forward<EventStreamId>(id));
   }

   template <typename EventStreamFactory, typename EventStreamId>
   requires (!skizzay::domains::tag_invocable<get_event_stream_function_, EventStreamFactory &&, EventStreamId>)
      && std::invocable<EventStreamFactory &&, EventStreamId>
      && event_stream_result<std::invoke_result_t<get_event_stream_function_ &&, EventStreamId>, EventStreamId>
   constexpr auto operator()(EventStreamFactory &&ef, EventStreamId id) const noexcept(
      std::is_nothrow_invocable_v<EventStreamFactory &&, EventStreamId>
   ) -> decltype(std::invoke_result_t<EventStreamFactory &&, EventStreamId>) {
      return std::invoke(std::forward<EventStreamFactory>(ef), std::forward<EventStreamId>(id));
   }

   template <typename EventStreamFactory, typename EventStreamId>
   requires (!skizzay::domains::tag_invocable<get_event_stream_function_, EventStreamFactory &&>)
      && requires(EventStreamFactory &&ef, EventStreamId &&id) {
            { std::forward<EventStreamFactory>(ef).get_event_stream(std::forward<EventStreamId>(id)) } -> event_stream_result<EventStreamId>;
         }
   constexpr auto operator()(EventStreamFactory &&ef, EventStreamId id) const noexcept(
      std::forward<EventStreamFactory>(ef).get_event_stream(std::forward<EventStreamId>(id))
   ) -> decltype(std::forward<EventStreamFactory>(ef).get_event_stream(std::forward<EventStreamId>(id))) {
      return std::forward<EventStreamFactory>(ef).get_event_stream(std::forward<EventStreamId>(id));
   }

   template <typename EventStreamFactory, typename EventStreamId>
   requires (!skizzay::domains::tag_invocable<get_event_stream_function_, EventStreamFactory &&, EventStreamId>)
      && skizzay::domains::concepts::dereferenceable<EventStreamFactory>
      && std::invocable<get_event_stream_function_, skizzay::domains::dereferenced_t<EventStreamFactory>, EventStreamId>
      && event_stream_result<std::invoke_result_t<get_event_stream_function_, skizzay::domains::dereferenced_t<EventStreamFactory>, EventStreamId>>
   constexpr auto operator()(EventStreamFactory &&ef, EventStreamId id) const noexcept(
      std::is_nothrow_invocable_v<get_event_stream_function_, skizzay::domains::dereferenced_t<EventStreamFactory>, EventStreamId>
   ) -> std::invoke_result_t<get_event_stream_function_, skizzay::domains::dereferenced_t<EventStreamFactory>, EventStreamId> {
      return std::invoke(*this, skizzay::domains::get_reference(ef), std::forward<EventStreamId>(id));
   }
} get_event_stream = {};

}

namespace concepts {
   template<typename EventStreamFactory, typename EventStreamId>
   concept event_stream_factory = identifier<EventStreamId>
      && requires (EventStreamFactory &&es, EventStreamId &&id) {
            { skizzay::domains::event_source::get_event_stream(std::forward<EventStreamFactory>(es), std::forward<EventStreamId>(id)) } -> event_stream;
            { event_stream_id_t<decltype(skizzay::domains::event_source::get_event_stream(std::forward<EventStreamFactory>(es), std::forward<EventStreamId>(id)))> } -> std::same_as<EventStreamId>
         };

#if 0
   && std::same_as<event_stream_id_t<EventRange>, event_stream_id_t<commit_t<EventStream, EventRange>>>
   && std::same_as<event_stream_timestamp_t<EventRange>, decltype(std::declval<commit_t<EventStream, EventRange>>().commit_timestamp())>
   && std::same_as<event_stream_sequence_t<EventRange>, decltype(std::declval<commit_t<EventStream, EventRange>>().event_stream_starting_sequence())>;
#endif

   template<typename EventStreamStore, typename EventStream, typename EventRange>
   concept event_stream_store = event_stream_factory<EventStreamStore, event_stream_id_t<EventStream>>
      && event_stream_writer<EventStream, EventRange>
      && requires (EventStreamStore &ess, EventStream &&es, EventRange &&er) {
            { ess.put_event_stream(std::forward<EventStream>(es), std::forward<EventRange>(er)) } -> commit;
         };
} // namespace concepts

} // namespace skizzay::domains::event_source