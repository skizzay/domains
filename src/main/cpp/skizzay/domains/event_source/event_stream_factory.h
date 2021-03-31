#pragma once

#include <skizzay/domains/event_source/commit.h>
#include <skizzay/domains/event_source/event_stream.h>

namespace skizzay::domains::event_source {

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
   concept event_stream_factory = identifier<EventStreamId> &&
      requires (EventStreamFactory &es, EventStreamId id) {
         { es.get_event_stream(id) } -> event_stream;
         { event_stream_id_t<decltype(es.get_event_stream(id))> } -> std::same_as<EventStreamId>
      };
} // namespace concepts

} // namespace skizzay::domains::event_source