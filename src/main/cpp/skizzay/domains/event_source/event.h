#pragma once

#include <skizzay/domains/event_source/concepts.h>
#include <skizzay/domains/event_source/tag_dispatch.h>

#include <skizzay/domains/event_source/concepts.h>
#include <skizzay/domains/event_source/tag_dispatch.h>
#include <functional>
#include <utility>
#include <variant>

namespace skizzay::domains::event_source {

inline namespace event_stream_id_details_ {
inline constexpr struct event_stream_id_function_ final {
   template <typename E>
   requires skizzay::domains::event_source::tag_invocable<event_stream_id_function_,
                                                          E const &> constexpr auto
   operator()(E const &e) const
      noexcept(noexcept(skizzay::domains::event_source::tag_invoke(*this,
                                                                   static_cast<E const &>(e))))
         -> decltype(skizzay::domains::event_source::tag_invoke(*this, static_cast<E const &>(e))) {
      return skizzay::domains::event_source::tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires requires(E const &e) {
      { e.event_stream_id() }
      ->concepts::identifier;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.event_stream_id()))
      -> decltype(e.event_stream_id()) {
      return e.event_stream_id();
   }
} event_stream_id = {};
} // namespace event_stream_id_details_

inline namespace event_stream_sequence_details_ {
inline constexpr struct event_stream_sequence_function_ final {
   template <typename E>
   requires skizzay::domains::event_source::tag_invocable<event_stream_sequence_function_,
                                                          E const &> constexpr auto
   operator()(E const &e) const
      noexcept(noexcept(skizzay::domains::event_source::tag_invoke(*this,
                                                                   static_cast<E const &>(e))))
         -> decltype(skizzay::domains::event_source::tag_invoke(*this, static_cast<E const &>(e))) {
      return skizzay::domains::event_source::tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires requires(E const &e) {
      { e.event_stream_sequence() }
      ->concepts::sequenced;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.event_stream_sequence()))
      -> decltype(e.event_stream_sequence()) {
      return e.event_stream_sequence();
   }
} event_stream_sequence = {};
} // namespace event_stream_sequence_details_

inline namespace event_stream_timestamp_details_ {
inline constexpr struct event_stream_timestamp_function_ final {
   template <typename E>
   requires skizzay::domains::event_source::tag_invocable<event_stream_timestamp_function_,
                                                          E const &> constexpr auto
   operator()(E const &e) const
      noexcept(noexcept(skizzay::domains::event_source::tag_invoke(*this,
                                                                   static_cast<E const &>(e))))
         -> decltype(skizzay::domains::event_source::tag_invoke(*this, static_cast<E const &>(e))) {
      return skizzay::domains::event_source::tag_invoke(*this, static_cast<E const &>(e));
   }
   template <typename E>
   requires requires(E const &e) {
      { e.event_stream_timestamp() }
      ->concepts::timestamp;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.event_stream_timestamp()))
      -> decltype(e.event_stream_timestamp()) {
      return e.event_stream_timestamp();
   }
} event_stream_timestamp = {};
} // namespace event_stream_timestamp_details_

template <concepts::identifier StreamIdType, concepts::sequenced StreamSequenceType,
          concepts::timestamp StreamTimestampType>
class basic_event {
   StreamIdType event_stream_id_;
   StreamSequenceType event_stream_sequence_;
   StreamTimestampType event_stream_timestamp_;

public:
   constexpr explicit basic_event(StreamIdType stream_id, StreamSequenceType stream_sequence,
                                  StreamTimestampType stream_timestamp) noexcept
      : event_stream_id_{std::move(stream_id)},
        event_stream_sequence_{std::move(stream_sequence)},
        event_stream_timestamp_{std::move(stream_timestamp)} {
   }

   constexpr auto event_stream_id() const
      noexcept(std::is_nothrow_copy_constructible_v<StreamIdType>) {
      return event_stream_id_;
   }

   constexpr auto event_stream_sequence() const
      noexcept(std::is_nothrow_copy_constructible_v<StreamSequenceType>) {
      return event_stream_sequence_;
   }

   constexpr auto event_stream_timestamp() const
      noexcept(std::is_nothrow_copy_constructible_v<StreamTimestampType>) {
      return event_stream_timestamp_;
   }
};

template <class Tag, concepts::identifier StreamIdType, concepts::sequenced StreamSequenceType,
          concepts::timestamp StreamTimestampType>
class tagged_event : public basic_event<StreamIdType, StreamSequenceType, StreamTimestampType> {
   using basic_event<StreamIdType, StreamSequenceType, StreamTimestampType>::basic_event;
};

namespace concepts {

template <typename T>
concept event = std::copyable<T> &&requires(T const &t) {
   { skizzay::domains::event_source::event_stream_id(t) }
   ->identifier;
   { skizzay::domains::event_source::event_stream_sequence(t) }
   ->sequenced;
   { skizzay::domains::event_source::event_stream_timestamp(t) }
   ->timestamp;
};

template <typename H, typename E>
concept event_handler = event<E> &&std::invocable<H, E const &>;

} // namespace concepts

inline namespace dispatch_event_details_ {
inline constexpr struct dispatch_event_function_ final {
   template <typename H, typename E>
   requires tag_invocable<dispatch_event_function_, H, E const &> constexpr auto
   operator()(H &&h, E const &e) const
      noexcept(nothrow_tag_invocable<dispatch_event_function_, H, E const &>) {
      return skizzay::domains::event_source::tag_invoke(*this, std::forward<H>(h), e);
   }

   template <concepts::event E, concepts::event_handler<E> H>
   requires std::invocable<H, E const &> constexpr std::invoke_result_t<H, E const &>
   operator()(H &&h, E const &e) const noexcept(std::is_nothrow_invocable_v<H, E const &>) {
      return std::invoke(std::forward<H>(h), e);
   }

   template <typename... E, typename H>
   constexpr auto operator()(H &&h, std::variant<E...> const &v) const {
      return std::visit([this, &h](auto const &e) { return (*this)(std::forward<H>(h), e); }, v);
   }

   template <typename P, typename H>
   requires requires(H &&h, P const &p) {
      {std::declval<dispatch_event_function_ const>()(std::forward<H>(h), *p)};
   }
   constexpr auto operator()(H &&h, P const &p) const
      noexcept(noexcept(std::declval<dispatch_event_function_ const>()(std::forward<H>(h), *p))) {
      return (*this)(std::forward<H>(h), *p);
   }
} dispatch_event = {};
} // namespace dispatch_event_details_

namespace concepts {
template <typename H, typename E>
concept event_dispatcher = requires(H &&h, E const &e){
   { skizzay::domains::event_source::dispatch_event(std::forward<H>(h), e) };
};
}

} // namespace skizzay::domains::event_source