#pragma once

#include <functional>
#include <ranges>
#include <skizzay/domains/concepts.h>
#include <skizzay/domains/tag_dispatch.h>
#include <utility>
#include <variant>

namespace skizzay::domains {

inline namespace event_stream_id_details_ {
inline constexpr struct event_stream_id_function_ final {
   template <typename E>
   requires tag_invocable<event_stream_id_function_, E const &>
      && concepts::identifier<
         tag_invoke_result_t<event_stream_id_function_, E const &>
      >
   constexpr auto operator()(E const &e) const noexcept(
      nothrow_tag_invocable<event_stream_id_function_, E const &>
   ) -> tag_invoke_result_t<event_stream_id_function_, E const &> {
      return tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires (!tag_invocable<event_stream_id_function_, E const &>)
      && requires(E const &e) {
      { e.event_stream_id() } -> concepts::identifier;
   }
   constexpr auto operator()(E const &e) const noexcept(
      noexcept(e.event_stream_id())
   ) -> decltype(e.event_stream_id()) {
      return e.event_stream_id();
   }

   template <typename E>
   requires requires(E const &e) {
      { *e };
      requires std::invocable<event_stream_id_function_, decltype(*e)>;
      requires concepts::identifier<std::invoke_result_t<event_stream_id_function_, decltype(*e)>>;
      requires !tag_invocable<event_stream_id_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const
      noexcept(std::is_nothrow_invocable_v<event_stream_id_function_, decltype(*e)>)
         -> std::invoke_result_t<event_stream_id_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }

   template<typename E, typename ...Es>
   requires (!tag_invocable<event_stream_id_function_, E const &>)
      && std::conjunction_v<
         std::is_same<
            std::invoke_result_t<event_stream_id_function_, E>,
            std::invoke_result_t<event_stream_id_function_, Es>
         >...
      >
      && concepts::identifier<std::invoke_result_t<event_stream_id_function_, E>>
   constexpr auto operator()(std::variant<E, Es...> const &e) const
   -> std::invoke_result_t<event_stream_id_function_, E> {
      return std::visit([this](auto const &concrete_event) -> std::invoke_result_t<event_stream_id_function_, E> {
         return std::invoke(*this, concrete_event);
      }, e);
   }
} event_stream_id = {};
} // namespace event_stream_id_details_


inline namespace event_stream_sequence_details_ {
inline constexpr struct event_stream_sequence_function_ final {
   template <typename E>
   requires tag_invocable<event_stream_sequence_function_,
                                            E const &> &&
   concepts::sequenced<tag_invoke_result_t<event_stream_sequence_function_, E const &>>
   constexpr auto
   operator()(E const &e) const
      noexcept(nothrow_tag_invocable<event_stream_sequence_function_, E const &>)
         -> tag_invoke_result_t<event_stream_sequence_function_, E const &> {
      return tag_invoke(*this, static_cast<E const &>(e));
   }

   template <typename E>
   requires requires(E const &e) {
      { e.event_stream_sequence() }
      ->concepts::sequenced;
      requires !tag_invocable<event_stream_sequence_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const noexcept(noexcept(e.event_stream_sequence()))
      -> decltype(e.event_stream_sequence()) {
      return e.event_stream_sequence();
   }

   template <typename E>
   requires (!tag_invocable<event_stream_sequence_function_, E const &>)
      && concepts::dereferenceable<E>
      && std::invocable<event_stream_sequence_function_, dereferenced_t<E>>
      && concepts::sequenced<std::invoke_result_t<event_stream_sequence_function_, dereferenced_t<E>>>
   constexpr auto operator()(E const &e) const noexcept(
      noexcept(std::is_nothrow_invocable_v<event_stream_sequence_function_, dereferenced_t<E>>)
   ) -> std::invoke_result_t<event_stream_sequence_function_, dereferenced_t<E>> {
      return std::invoke(*this, *e);
   }

   template<typename E, typename ...Es>
   requires (!tag_invocable<event_stream_sequence_function_, E const &>)
      && std::conjunction_v<
         std::is_same<
            std::invoke_result_t<event_stream_sequence_function_, E>,
            std::invoke_result_t<event_stream_sequence_function_, Es>
         >...
      >
      && concepts::sequenced<std::invoke_result_t<event_stream_sequence_function_, E>>
   constexpr auto operator()(std::variant<E, Es...> const &e) const
   -> std::invoke_result_t<event_stream_sequence_function_, E> {
      return std::visit([this](auto const &concrete_event) -> std::invoke_result_t<event_stream_sequence_function_, E> {
         return std::invoke(*this, concrete_event);
      }, e);
   }
} event_stream_sequence = {};
} // namespace event_stream_sequence_details_


inline namespace event_stream_timestamp_details_ {
inline constexpr struct event_stream_timestamp_function_ final {
   template <typename E>
   requires tag_invocable<event_stream_timestamp_function_, E const &>
      && concepts::timestamp<tag_invoke_result_t<event_stream_timestamp_function_, E const &>>
   constexpr auto operator()(E const &e) const
      noexcept(noexcept(tag_invoke(*this, static_cast<E const &>(e))))
         -> decltype(tag_invoke(*this, static_cast<E const &>(e))) {
      return tag_invoke(*this, static_cast<E const &>(e));
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
      requires !tag_invocable<event_stream_timestamp_function_, E const &>;
   }
   constexpr auto operator()(E const &e) const noexcept(
      noexcept(std::is_nothrow_invocable_v<event_stream_timestamp_function_, decltype(*e)>))
      -> std::invoke_result_t<event_stream_timestamp_function_, decltype(*e)> {
      return std::invoke(*this, *e);
   }

   template<typename E, typename ...Es>
   requires (!tag_invocable<event_stream_timestamp_function_, E const &>)
      && std::conjunction_v<
         std::is_same<
            std::invoke_result_t<event_stream_timestamp_function_, E>,
            std::invoke_result_t<event_stream_timestamp_function_, Es>
         >...
      >
      && concepts::timestamp<std::invoke_result_t<event_stream_timestamp_function_, E>>
   constexpr auto operator()(std::variant<E, Es...> const &e) const
   -> std::invoke_result_t<event_stream_timestamp_function_, E> {
      return std::visit([this](auto const &concrete_event) -> std::invoke_result_t<event_stream_timestamp_function_, E> {
         return std::invoke(*this, concrete_event);
      }, e);
   }
} event_stream_timestamp = {};
} // namespace event_stream_timestamp_details_


inline namespace with_timestamp_details_ {
inline constexpr struct with_timestamp_function_ final {
   template <typename E>
   using timestamp_t = std::invoke_result_t<tag_t<event_stream_timestamp>, std::remove_reference_t<E> const &>;

   template <typename E>
   requires tag_invocable<with_timestamp_function_, E &, timestamp_t<E>>
      && std::same_as<E, std::remove_const_t<E>>
      && concepts::timestamp<timestamp_t<E>>
   constexpr E & operator()(E &e, timestamp_t<E> t) const noexcept(
      nothrow_tag_invocable<with_timestamp_function_, E &, timestamp_t<E>>
   ) {
      tag_invoke(*this, static_cast<E &>(e), t);
      return e;
   }

   template <typename E>
   requires (!tag_invocable<with_timestamp_function_, E &, timestamp_t<E>>)
      && std::same_as<E, std::remove_const_t<E>>
      && concepts::timestamp<timestamp_t<E>>
      && requires(E &e, timestamp_t<E> t) {
         { e.with_timestamp(t) };
      }
   constexpr E & operator()(E &e, timestamp_t<E> t) const noexcept(
      noexcept(e.with_timestamp(t))
   ) {
      e.with_timestamp(t);
      return e;
   }

   template<typename E, typename ...Es>
   requires (!tag_invocable<with_timestamp_function_, std::variant<E, Es...> &, timestamp_t<E>>)
      && std::conjunction_v<
         std::is_same<
            std::invoke_result_t<tag_t<event_stream_timestamp>, E>,
            std::invoke_result_t<tag_t<event_stream_timestamp>, Es>
         >...
      >
   constexpr std::variant<E, Es...> & operator()(std::variant<E, Es...> &v, timestamp_t<E> t) const {
      std::visit([this, t=std::move(t)](auto &concrete_event) {
         std::invoke(*this, concrete_event, std::move(t));
      }, v);
      return v;
   }
} with_timestamp = {};
} // namespace with_timestamp_details_

namespace concepts {

template <typename T>
concept event = std::copyable<std::remove_cvref_t<T>> && requires(std::remove_cvref_t<T> const &t) {
   { event_stream_id(t) }
   -> concepts::identifier;
   { event_stream_sequence(t) }
   -> concepts::sequenced;
   { event_stream_timestamp(t) }
   -> concepts::timestamp;
};

template <typename T>
concept event_range = std::ranges::range<T> && event<std::ranges::range_value_t<T>>;

template <typename EventRange, typename Event>
concept event_range_of = event_range<EventRange>
   && event<Event>
   && std::same_as<Event, std::ranges::range_value_t<EventRange>>;

} // namespace concepts

namespace details_ {

template<typename, typename=void> struct event_stream_id_type_impl;

template<typename T>
struct event_stream_id_type_impl<T, std::void_t<decltype(event_stream_id(std::declval<T>()))>> {
   using type = decltype(event_stream_id(std::declval<T>()));
};

template<concepts::event_range EventRange>
struct event_stream_id_type_impl<EventRange, std::void_t<>> {
   using type = typename event_stream_id_type_impl<std::ranges::range_value_t<EventRange>>::type;
};

template<typename, typename=void> struct event_stream_sequence_type_impl;

template<typename Event>
struct event_stream_sequence_type_impl<Event, std::void_t<decltype(event_stream_sequence(std::declval<Event>()))>> {
   using type = decltype(event_stream_sequence(std::declval<Event>()));
};

template<concepts::event_range EventRange>
struct event_stream_sequence_type_impl<EventRange, std::void_t<>> {
   using type = typename event_stream_sequence_type_impl<std::ranges::range_value_t<EventRange>>::type;
};

template<typename T>
   requires concepts::event<typename T::event_type>
struct event_stream_sequence_type_impl<T, std::void_t<typename T::event_type>> : event_stream_sequence_type_impl<typename T::event_type> {
};

template<typename, typename=void> struct event_stream_timestamp_type_impl;

template<typename Event>
struct event_stream_timestamp_type_impl<Event, std::void_t<decltype(event_stream_timestamp(std::declval<Event>()))>> {
   using type = decltype(event_stream_timestamp(std::declval<Event>()));
};

template<concepts::event_range EventRange>
struct event_stream_timestamp_type_impl<EventRange, std::void_t<>> : event_stream_timestamp_type_impl<std::ranges::range_value_t<EventRange>> {
};

template<typename T>
   requires concepts::event<typename T::event_type>
struct event_stream_timestamp_type_impl<T, std::void_t<typename T::event_type>> : event_stream_timestamp_type_impl<typename T::event_type> {
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

namespace concepts {

template<typename L, typename R>
concept event_type_compatible = same_reference_removed<event_stream_id_t<L>, event_stream_id_t<R>>
   && same_reference_removed<event_stream_sequence_t<L>, event_stream_sequence_t<R>>
   && same_reference_removed<event_stream_timestamp_t<L>, event_stream_timestamp_t<R>>;
}


template <
   concepts::identifier StreamIdType,
   concepts::sequenced StreamSequenceType,
   concepts::timestamp StreamTimestampType
>
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

   constexpr basic_event &with_timestamp(StreamTimestampType stream_timestamp) noexcept {
      event_stream_timestamp_ = std::move(stream_timestamp);
      return *this;
   }
};

template <
   class Tag,
   concepts::identifier StreamIdType,
   concepts::sequenced StreamSequenceType,
   concepts::timestamp StreamTimestampType
>
struct tagged_event : public basic_event<StreamIdType, StreamSequenceType, StreamTimestampType> {
   using basic_event<StreamIdType, StreamSequenceType, StreamTimestampType>::basic_event;
};

namespace details_ {

template<typename>
struct is_variant_impl : std::false_type {
};

template<typename ...Ts>
struct is_variant_impl<std::variant<Ts...>> : std::true_type {
};

template<typename T>
concept is_variant = is_variant_impl<T>::value;

}

inline namespace dispatch_event_details_ {
inline constexpr struct dispatch_event_function_ final {

   template <typename H, typename E>
   requires tag_invocable<dispatch_event_function_, H, E const &>
   constexpr auto operator()(H &&h, E const &e) const
      noexcept(nothrow_tag_invocable<dispatch_event_function_, H, E const &>) {
      return tag_invoke(*this, std::forward<H>(h), e);
   }

   template <typename H, concepts::event... E>
   requires (!tag_invocable<dispatch_event_function_, H, std::variant<E...> const &>)
      && std::conjunction_v<std::is_invocable<dispatch_event_function_, H, E const &>...>
   constexpr auto operator()(H &&h, std::variant<E...> const &v) const {
      return std::visit([this, &h](concepts::event auto const &e) {
         return std::invoke(*this, std::forward<H>(h), e);
         }, v);
   }

   template <typename H, concepts::event E>
   requires (!tag_invocable<dispatch_event_function_, H, E const &>)
      && std::invocable<H, E const &>
      && (!details_::is_variant<E>)
   constexpr auto operator()(H &&h, E const &e) const noexcept(
      std::is_nothrow_invocable_v<H, E const &>
   ) -> std::invoke_result_t<H, E const &>
   {
      return std::invoke(std::forward<H>(h), e);
   }

   template <typename H, concepts::event E>
   requires (!tag_invocable<dispatch_event_function_, H, E const &>)
      && (!std::invocable<H, E const &>)
      && std::invocable<dispatch_event_function_, H, dereferenced_t<E>>
   constexpr auto operator()(H &&h, E const &e) const noexcept(std::is_nothrow_invocable_v<dispatch_event_function_, H, decltype(*e)>) {
      return std::invoke(*this, std::forward<H>(h), get_reference(e));
   }
} dispatch_event = {};
} // namespace dispatch_event_details_

namespace concepts {
template <typename H, typename E>
concept event_dispatcher = requires(H &&h, E const &e) {
   {dispatch_event(std::forward<H>(h), e)};
};
} // namespace concepts

} // namespace event_source