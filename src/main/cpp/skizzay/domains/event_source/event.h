#pragma once

#include <functional>
#include <skizzay/domains/event_source/concepts.h>
#include <skizzay/domains/tag_dispatch.h>
#include <utility>
#include <variant>

namespace skizzay::domains::event_source {

inline namespace event_stream_id_details_ {
inline constexpr struct event_stream_id_function_ final {
   template <typename E>
   requires skizzay::domains::tag_invocable<event_stream_id_function_, E const &> constexpr auto
   operator()(E const &e) const
      noexcept(skizzay::domains::nothrow_tag_invocable<event_stream_id_function_, E const &>)
         -> skizzay::domains::tag_invoke_result_t<event_stream_id_function_, E const &> {
      return skizzay::domains::tag_invoke(*this, static_cast<E const &>(e));
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
   requires skizzay::domains::tag_invocable<event_stream_sequence_function_,
                                            E const &> constexpr auto
   operator()(E const &e) const
      noexcept(skizzay::domains::nothrow_tag_invocable<event_stream_sequence_function_, E const &>)
         -> skizzay::domains::tag_invoke_result_t<event_stream_sequence_function_, E const &> {
      return skizzay::domains::tag_invoke(*this, static_cast<E const &>(e));
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
   requires skizzay::domains::tag_invocable<event_stream_timestamp_function_,
                                            E const &> constexpr auto
   operator()(E const &e) const
      noexcept(noexcept(skizzay::domains::tag_invoke(*this, static_cast<E const &>(e))))
         -> decltype(skizzay::domains::tag_invoke(*this, static_cast<E const &>(e))) {
      return skizzay::domains::tag_invoke(*this, static_cast<E const &>(e));
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

} // namespace concepts

inline namespace get_dispatchable_event_details_ {
inline constexpr struct get_dispatchable_event_function_ final {
   template <typename E>
   constexpr E const &operator()(E const &e) const noexcept {
      return e;
   }

   template <typename E>
   requires requires(E const &e) {
      {*e};
      requires std::invocable<get_dispatchable_event_function_, decltype(*e)>;
   }
   constexpr auto operator()(E const &e) const
      noexcept(std::is_nothrow_invocable_v<get_dispatchable_event_function_, decltype(*e)>) {
      return std::invoke(*this, *e);
   }

   template <typename E>
   requires skizzay::domains::tag_invocable<get_dispatchable_event_function_,
                                            E const &> constexpr auto
   operator()(E const &e) const noexcept(noexcept(skizzay::domains::tag_invoke(*this, e))) {
      return skizzay::domains::tag_invoke(*this, e);
   }
} get_dispatchable_event = {};
} // namespace get_dispatchable_event_details_

namespace concepts {
template <typename T>
concept dispatchable_event = requires(T const &t) {
   {skizzay::domains::event_source::get_dispatchable_event(t)};
   requires event<
      std::remove_cvref_t<decltype(skizzay::domains::event_source::get_dispatchable_event(t))>>;
};
} // namespace concepts

inline namespace dispatch_event_details_ {
inline constexpr struct dispatch_event_function_ final {
   template <typename... T>
   using variant_t = std::variant<T...>;

   template <typename T>
   using get_dispatchable_event_result_t =
      decltype(skizzay::domains::event_source::get_dispatchable_event(std::declval<T const &>()));

   template <typename H, typename E>
   requires tag_invocable<dispatch_event_function_, H,
                          get_dispatchable_event_result_t<E> const &> constexpr auto
   operator()(H &&h, E const &e) const
      noexcept(nothrow_tag_invocable<dispatch_event_function_, H,
                                     get_dispatchable_event_result_t<E> const &>) {
      return skizzay::domains::tag_invoke(
         *this, std::forward<H>(h), skizzay::domains::event_source::get_dispatchable_event(e));
   }

   template <typename H, concepts::dispatchable_event E>
   requires std::invocable<H, get_dispatchable_event_result_t<E> const &> constexpr auto
   operator()(H &&h, E const &e) const
      noexcept(std::is_nothrow_invocable_v<H, get_dispatchable_event_result_t<E> const &>) {
      return std::invoke(std::forward<H>(h),
                         skizzay::domains::event_source::get_dispatchable_event(e));
   }

   template <typename H, typename... E>
   constexpr auto operator()(H &&h, std::variant<E...> const &v) const {
      return std::visit(
         [this, &h](auto const &e) { return std::invoke(*this, std::forward<H>(h), e); }, v);
   }

   template <typename H, typename E>
   requires requires(E const &e) {
      {*e};
      requires std::invocable<dispatch_event_function_, H, decltype(*e)>;
      requires !concepts::dispatchable_event<E>;
   }
   constexpr auto operator()(H &&h, E const &e) const {
      return std::invoke(*this, std::forward<H>(h), *e);
   }
} dispatch_event = {};
} // namespace dispatch_event_details_

namespace concepts {
template <typename H, typename E>
concept event_dispatcher = requires(H &&h, E const &e) {
   {skizzay::domains::event_source::dispatch_event(std::forward<H>(h), e)};
};

template <typename T>
concept event_range = std::ranges::range<T> && event<std::ranges::range_value_t<T>>;

template <typename T>
concept dispatchable_event_range =
   std::ranges::range<T> &&dispatchable_event<std::ranges::range_value_t<T>>;
} // namespace concepts

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

template <concepts::dispatchable_event... E>
struct variant_event : public std::variant<E...> {
   using event_stream_id_type =
      std::common_type_t<decltype(skizzay::domains::event_source::event_stream_id(
         skizzay::domains::event_source::get_dispatchable_event(std::declval<E>())))...>;
   using event_stream_sequence_type =
      std::common_type_t<decltype(skizzay::domains::event_source::event_stream_sequence(
         skizzay::domains::event_source::get_dispatchable_event(std::declval<E>())))...>;
   using event_stream_timestamp_type =
      std::common_type_t<decltype(skizzay::domains::event_source::event_stream_timestamp(
         skizzay::domains::event_source::get_dispatchable_event(std::declval<E>())))...>;

   constexpr event_stream_id_type event_stream_id() const {
      return std::visit(
         [](concepts::dispatchable_event auto const &e) {
            return skizzay::domains::event_source::event_stream_id(
               skizzay::domains::event_source::get_dispatchable_event(e));
         },
         *this);
   }

   constexpr event_stream_sequence_type event_stream_sequence() const {
      return std::visit(
         [](concepts::dispatchable_event auto const &e) {
            return skizzay::domains::event_source::event_stream_sequence(
               skizzay::domains::event_source::get_dispatchable_event(e));
         },
         *this);
   }

   constexpr event_stream_timestamp_type event_stream_timestamp() const {
      return std::visit(
         [](concepts::dispatchable_event auto const &e) {
            return skizzay::domains::event_source::event_stream_timestamp(
               skizzay::domains::event_source::get_dispatchable_event(e));
         },
         *this);
   }
};

} // namespace skizzay::domains::event_source