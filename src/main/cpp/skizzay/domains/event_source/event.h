#pragma once

#include <functional>
#include <ranges>
#include <skizzay/domains/event_source/concepts.h>
#include <skizzay/domains/tag_dispatch.h>
#include <utility>
#include <variant>

namespace skizzay::domains::event_source {

inline namespace event_stream_id_details_ {
inline constexpr struct event_stream_id_function_ final {
   template <typename E>
   requires skizzay::domains::tag_invocable<event_stream_id_function_, E const &> &&
      concepts::identifier<
         skizzay::domains::tag_invoke_result_t<event_stream_id_function_, E const &>>
   constexpr auto
      operator()(E const &e) const
      noexcept(skizzay::domains::nothrow_tag_invocable<event_stream_id_function_, E const &>)
         -> skizzay::domains::tag_invoke_result_t<event_stream_id_function_, E const &> {
      return skizzay::domains::tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires requires(E const &e) {
      { e.event_stream_id() }
      ->concepts::identifier;
      requires !skizzay::domains::tag_invocable<event_stream_id_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.event_stream_id()))
      -> decltype(e.event_stream_id()) {
      return e.event_stream_id();
   }

   template <typename E>
   requires requires(E const &e) {
      { *e };
      requires std::invocable<event_stream_id_function_, decltype(*e)>;
      requires concepts::identifier<std::invoke_result_t<event_stream_id_function_, decltype(*e)>>;
      requires !skizzay::domains::tag_invocable<event_stream_id_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const
      noexcept(std::is_nothrow_invocable_v<event_stream_id_function_, decltype(*e)>)
         -> std::invoke_result_t<event_stream_id_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }
} event_stream_id = {};
} // namespace event_stream_id_details_


inline namespace event_stream_sequence_details_ {
inline constexpr struct event_stream_sequence_function_ final {
   template <typename E>
   requires skizzay::domains::tag_invocable<event_stream_sequence_function_,
                                            E const &> &&
   concepts::sequenced<skizzay::domains::tag_invoke_result_t<event_stream_sequence_function_, E const &>>
   constexpr auto
   operator()(E const &e) const
      noexcept(skizzay::domains::nothrow_tag_invocable<event_stream_sequence_function_, E const &>)
         -> skizzay::domains::tag_invoke_result_t<event_stream_sequence_function_, E const &> {
      return skizzay::domains::tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires requires(E const &e) {
      { e.event_stream_sequence() }
      ->concepts::sequenced;
      requires !skizzay::domains::tag_invocable<event_stream_sequence_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.event_stream_sequence()))
      -> decltype(e.event_stream_sequence()) {
      return e.event_stream_sequence();
   }

   template <typename E>
   requires requires(E const &e) {
      {*e};
      requires std::invocable<event_stream_sequence_function_, decltype(*e)>;
      requires concepts::sequenced<
         std::invoke_result_t<event_stream_sequence_function_, decltype(*e)>>;
      requires !skizzay::domains::tag_invocable<event_stream_sequence_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const
      noexcept(noexcept(std::is_nothrow_invocable_v<event_stream_sequence_function_, decltype(*e)>))
         -> std::invoke_result_t<event_stream_sequence_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }
} event_stream_sequence = {};
} // namespace event_stream_sequence_details_


inline namespace event_stream_timestamp_details_ {
inline constexpr struct event_stream_timestamp_function_ final {
   template <typename E>
   requires skizzay::domains::tag_invocable<event_stream_timestamp_function_, E const &>
      && concepts::timestamp<skizzay::domains::tag_invoke_result_t<event_stream_timestamp_function_, E const &>>
   constexpr auto operator()(E const &e) const
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

   template <typename E>
   requires requires(E const &e) {
      {*e};
      requires std::invocable<event_stream_timestamp_function_, decltype(*e)>;
      requires concepts::timestamp<
         std::invoke_result_t<event_stream_timestamp_function_, decltype(*e)>>;
      requires !skizzay::domains::tag_invocable<event_stream_timestamp_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const noexcept(
      noexcept(std::is_nothrow_invocable_v<event_stream_timestamp_function_, decltype(*e)>))
      -> std::invoke_result_t<event_stream_timestamp_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }
} event_stream_timestamp = {};
} // namespace event_stream_timestamp_details_


inline namespace with_timestamp_details_ {
inline constexpr struct with_timestamp_function_ final {
   template <typename E>
   using timestamp_t = std::invoke_result_t<decltype(skizzay::domains::event_source::event_stream_timestamp), E &>;

   template <typename E>
   requires skizzay::domains::tag_invocable<with_timestamp_function_, E &, timestamp_t<E>>
      && concepts::timestamp<timestamp_t<E>>
   constexpr auto operator()(E &e, timestamp_t<E> t) const
      noexcept(skizzay::domains::nothrow_tag_invocable<with_timestamp_function_, E &, timestamp_t<E>>)
         -> skizzay::domains::tag_invoke_result_t<with_timestamp_function_, E &, timestamp_t<E>> {
      return skizzay::domains::tag_invoke(*this, static_cast<E &>(e), t);
   }

   template <typename E>
   requires (!skizzay::domains::tag_invocable<with_timestamp_function_, E &, timestamp_t<E>>)
      && concepts::timestamp<timestamp_t<E>>
      && requires(E &e, timestamp_t<E> t) {
         { e.with_timestamp(t) };
      }
   constexpr auto operator()(E &e, timestamp_t<E> t) const
      noexcept(noexcept(e.with_timestamp(t))) -> decltype(e.with_timestamp(t)) {
      return e.with_timestamp(t);
   }
} with_timestamp = {};
} // namespace event_stream_timestamp_details_

namespace concepts {

template <typename T>
concept event = std::copyable<std::remove_cvref_t<T>> && requires(std::remove_cvref_t<T> const &t) {
   { skizzay::domains::event_source::event_stream_id(t) }
   ->identifier;
   { skizzay::domains::event_source::event_stream_sequence(t) }
   ->sequenced;
   { skizzay::domains::event_source::event_stream_timestamp(t) }
   ->timestamp;
};

template <typename T>
concept event_range = std::ranges::range<T> && event<std::ranges::range_value_t<T>>;

} // namespace concepts

namespace details_ {

template<typename, typename=void> struct event_stream_id_type_impl;

template<typename T>
struct event_stream_id_type_impl<T, std::void_t<decltype(skizzay::domains::event_source::event_stream_id(std::declval<T>()))>> {
   using type = decltype(skizzay::domains::event_source::event_stream_id(std::declval<T>()));
};

template<concepts::event_range EventRange>
struct event_stream_id_type_impl<EventRange, std::void_t<>> {
   using type = typename event_stream_id_type_impl<std::ranges::range_value_t<EventRange>>::type;
};

template<typename, typename=void> struct event_stream_sequence_type_impl;

template<typename Event>
struct event_stream_sequence_type_impl<Event, std::void_t<decltype(skizzay::domains::event_source::event_stream_sequence(std::declval<Event>()))>> {
   using type = decltype(skizzay::domains::event_source::event_stream_sequence(std::declval<Event>()));
};

template<concepts::event_range EventRange>
struct event_stream_sequence_type_impl<EventRange, std::void_t<>> {
   using type = typename event_stream_sequence_type_impl<std::ranges::range_value_t<EventRange>>::type;
};

template<typename, typename=void> struct event_stream_timestamp_type_impl;

template<typename Event>
struct event_stream_timestamp_type_impl<Event, std::void_t<decltype(skizzay::domains::event_source::event_stream_timestamp(std::declval<Event>()))>> {
   using type = decltype(skizzay::domains::event_source::event_stream_timestamp(std::declval<Event>()));
};

template<concepts::event_range EventRange>
struct event_stream_timestamp_type_impl<EventRange, std::void_t<>> : event_stream_timestamp_type_impl<std::ranges::range_value_t<EventRange>> {
};

template<typename,typename=void> struct event_type_impl;

template<concepts::event Event>
struct event_type_impl<Event, std::void_t<>> {
   using type = Event;
};

template<concepts::event_range EventRange>
struct event_type_impl<EventRange, std::void_t<>> : event_type_impl<std::ranges::range_value_t<EventRange>> {
};

template<typename T>
struct event_type_impl<T, std::void_t<typename T::event_type>> : event_type_impl<typename T::event_type> {
};
} // namespace details_


template<typename T>
using event_t = typename details_::event_type_impl<T>::type;

template<typename T>
using event_stream_id_t = typename details_::event_stream_id_type_impl<T>::type;

template<typename T>
using event_stream_sequence_t = typename details_::event_stream_sequence_type_impl<T>::type;

template<typename T>
using event_stream_timestamp_t = typename details_::event_stream_timestamp_type_impl<T>::type;


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
        event_stream_timestamp_{std::move(stream_timestamp)}
   {
   }

   constexpr explicit basic_event(StreamIdType stream_id, StreamSequenceType stream_sequence) noexcept
      : basic_event{std::move(stream_id), std::move(stream_sequence), {}}
   {
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

   constexpr basic_event with_timestamp(StreamTimestampType stream_timestamp) const noexcept {
      return {event_stream_id_, event_stream_sequence_, std::move(stream_timestamp)};
   }

   constexpr basic_event &with_timestamp(StreamTimestampType stream_timestamp) noexcept {
      event_stream_timestamp_ = std::move(stream_timestamp);
      return *this;
   }
};

template <class Tag, concepts::identifier StreamIdType, concepts::sequenced StreamSequenceType,
          concepts::timestamp StreamTimestampType>
struct tagged_event : public basic_event<StreamIdType, StreamSequenceType, StreamTimestampType> {
   using basic_event<StreamIdType, StreamSequenceType, StreamTimestampType>::basic_event;

   constexpr tagged_event with_timestamp(StreamTimestampType stream_timestamp) const noexcept {
      return tagged_event{this->event_stream_id(), this->event_stream_sequence(), std::move(stream_timestamp)};
   }

   constexpr tagged_event &with_timestamp(StreamTimestampType stream_timestamp) noexcept {
      return static_cast<tagged_event<Tag, StreamIdType, StreamSequenceType, StreamTimestampType> &>(
         static_cast<basic_event<StreamIdType, StreamSequenceType, StreamTimestampType> &>(*this)
            .with_timestamp(std::move(stream_timestamp))
      );
   }
};

template <concepts::event... E>
struct variant_event : public std::variant<E...> {
   using event_stream_id_type =
      std::common_type_t<event_stream_id_t<E>...>;
   using event_stream_sequence_type =
      std::common_type_t<event_stream_sequence_t<E>...>;
   using event_stream_timestamp_type =
      std::common_type_t<event_stream_timestamp_t<E>...>;

   using std::variant<E...>::variant;

   constexpr event_stream_id_type event_stream_id() const {
      return std::visit(
         [](concepts::event auto const &e) {
            return skizzay::domains::event_source::event_stream_id(e);
         },
         *this);
   }

   constexpr event_stream_sequence_type event_stream_sequence() const {
      return std::visit(
         [](concepts::event auto const &e) {
            return skizzay::domains::event_source::event_stream_sequence(e);
         },
         *this);
   }

   constexpr event_stream_timestamp_type event_stream_timestamp() const {
      return std::visit(
         [](concepts::event auto const &e) {
            return skizzay::domains::event_source::event_stream_timestamp(e);
         },
         *this);
   }

   constexpr variant_event with_timestamp(event_stream_timestamp_type stream_timestamp) const {
      return std::visit(
         [stream_timestamp=std::move(stream_timestamp)] (concepts::event auto const &e) {
            return variant_event<E...>{skizzay::domains::event_source::with_timestamp(e, std::move(stream_timestamp))};
         },
         *this
      );
   }

   constexpr variant_event & with_timestamp(event_stream_timestamp_type stream_timestamp) {
      return std::visit(
         [stream_timestamp=std::move(stream_timestamp), this] (concepts::event auto &e) {
            skizzay::domains::event_source::with_timestamp(e, std::move(stream_timestamp));
            return *this;
         },
         *this
      );
   }
};

inline namespace dispatch_event_details_ {
inline constexpr struct dispatch_event_function_ final {
   template <typename... T>
   using variant_t = std::variant<T...>;

   template <typename H, typename E>
   requires tag_invocable<dispatch_event_function_, H, E const &>
   constexpr auto operator()(H &&h, E const &e) const
      noexcept(nothrow_tag_invocable<dispatch_event_function_, H, E const &>) {
      return skizzay::domains::tag_invoke(*this, std::forward<H>(h), e);
   }

   template <typename H, concepts::event... E>
   requires (!tag_invocable<dispatch_event_function_, H, std::variant<E...> const &>)
   constexpr auto operator()(H &&h, std::variant<E...> const &v) const {
      return std::visit([this, &h](concepts::event auto const &e) {
         return std::invoke(*this, std::forward<H>(h), e);
         }, static_cast<std::variant<E...> const &>(v));
   }

   template <typename H, concepts::event E>
   requires (!tag_invocable<dispatch_event_function_, H, E const &> && std::invocable<H, E const &>)
   constexpr auto operator()(H &&h, E const &e) const
      noexcept(std::is_nothrow_invocable_v<H, E const &>) {
      return std::invoke(std::forward<H>(h), e);
   }

   template <typename H, concepts::event E>
   requires requires(E const &e) {
      requires !tag_invocable<dispatch_event_function_, H, E const &>;
      requires !std::invocable<H, E const &>;
      {*e};
      requires std::invocable<dispatch_event_function_, H, decltype(*e)>;
   }
   constexpr auto operator()(H &&h, E const &e) const noexcept(std::is_nothrow_invocable_v<dispatch_event_function_, H, decltype(*e)>) {
      return std::invoke(*this, std::forward<H>(h), *e);
   }
} dispatch_event = {};
} // namespace dispatch_event_details_

namespace concepts {
template <typename H, typename E>
concept event_dispatcher = requires(H &&h, E const &e) {
   {skizzay::domains::event_source::dispatch_event(std::forward<H>(h), e)};
};
} // namespace concepts

} // namespace skizzay::domains::event_source