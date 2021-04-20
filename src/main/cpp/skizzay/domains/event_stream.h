#pragma once

#include <skizzay/domains/event.h>
#include <skizzay/domains/commit.h>
#include <skizzay/domains/concurrency_collision.h>
#include <functional>
#include <limits>
#include <mutex>
#include <ranges>
#include <sstream>

namespace skizzay::domains {
template<concepts::sequenced Sequence>
inline constexpr Sequence max_ending_exclusive_version{std::numeric_limits<typename Sequence::value_type>::max()};

inline namespace events_details_ {
inline constexpr struct events_function_ final {
   template <typename EventStream>
   constexpr auto operator()(EventStream const &e) const noexcept(
      std::is_nothrow_invocable_v<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>
   ) -> std::invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> {
      return std::invoke(*this, static_cast<EventStream const &>(e), event_stream_sequence_t<EventStream>{1}, max_ending_exclusive_version<event_stream_sequence_t<EventStream>>);
   }

   template <typename EventStream>
   requires
      tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> &&
      concepts::event_range<tag_invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>>
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> starting_inclusive_sequence, event_stream_sequence_t<EventStream> ending_exclusive_sequence) const
      noexcept(nothrow_tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>)
         -> tag_invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> {
      return tag_invoke(*this, static_cast<EventStream const &>(e), starting_inclusive_sequence, ending_exclusive_sequence);
   }

   template <typename EventStream>
   requires (!tag_invocable<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>)
      && requires(EventStream const &e, event_stream_sequence_t<EventStream> s) {
         { e.events(s, s.next()) } -> concepts::event_range;
      }
   constexpr auto operator()(EventStream const &e, event_stream_sequence_t<EventStream> starting_inclusive_sequence, event_stream_sequence_t<EventStream> ending_exclusive_sequence) const noexcept(noexcept(e.events(starting_inclusive_sequence, ending_exclusive_sequence)))
      -> decltype(e.events(starting_inclusive_sequence, ending_exclusive_sequence)) {
      return e.events(starting_inclusive_sequence, ending_exclusive_sequence);
   }
} events = {};
} // namespace events_details_

inline namespace event_stream_head_details_ {
inline constexpr struct event_stream_head_function_ final {

   template <typename EventStream>
   requires tag_invocable<event_stream_head_function_, EventStream const &>
      && concepts::event_stream_head<tag_invoke_result_t<event_stream_head_function_, EventStream const &>>
   constexpr auto operator()(EventStream const &e) const
      noexcept(nothrow_tag_invocable<event_stream_head_function_, EventStream const &>)
         -> tag_invoke_result_t<event_stream_head_function_, EventStream const &> {
      return tag_invoke(*this, static_cast<EventStream const &>(e));
   }

   template <typename EventStream>
   requires (!tag_invocable<event_stream_head_function_, EventStream const &>)
      && requires(EventStream const &e) {
         { e.event_stream_head() } -> concepts::event_stream_head;
      }
   constexpr auto operator()(EventStream const &e)
      -> std::tuple<event_stream_id_t<EventStream>, event_stream_sequence_t<EventStream>, event_stream_timestamp_t<EventStream>> {
      return e.event_stream_head();
   }

   template <typename EventStream>
   requires (!tag_invocable<event_stream_head_function_, EventStream const &>)
      && concepts::dereferenceable<EventStream>
   constexpr auto operator()(EventStream const &e)
      -> std::tuple<event_stream_id_t<EventStream>, event_stream_sequence_t<EventStream>, event_stream_timestamp_t<EventStream>> {
         return std::invoke(*this, get_reference(e));
   }

   template <typename EventStream>
   requires (!tag_invocable<event_stream_head_function_, EventStream const &>)
   constexpr auto operator()(EventStream const &e)
      -> std::tuple<event_stream_id_t<EventStream>, event_stream_sequence_t<EventStream>, event_stream_timestamp_t<EventStream>> {
      return std::make_tuple(
         event_stream_id(e),
         event_stream_sequence(e),
         event_stream_timestamp(e)
      );
   }
} event_stream_head = {};
} // namespace events_details_

inline namespace put_events_details_ {
inline constexpr struct put_events_function_ final {
   template<typename EventStream, concepts::event_range EventRange>
   requires
      tag_invocable<put_events_function_, EventStream &&, EventRange &&> &&
      concepts::commit<tag_invoke_result_t<put_events_function_, EventStream &, EventRange &&>>
   constexpr auto operator()(EventStream &&es, EventRange &&er) const noexcept(
      nothrow_tag_invocable<put_events_function_, EventStream &, EventRange &&>
   ) {
      return tag_invoke(*this, std::forward<EventStream>(es), std::forward<EventRange>(er));
   }

   template<typename EventStream, concepts::event_range EventRange>
   requires (!tag_invocable<put_events_function_, EventStream &&, EventRange &&>)
      && requires(EventStream &&es, EventRange &&er) {
         { std::forward<EventStream>(es).put_events(er) } -> concepts::commit;
      }
   constexpr auto operator()(EventStream &&es, EventRange &&er) const noexcept(
      noexcept(std::forward<EventStream>(es).put_events(std::forward<EventRange>(er)))
   ) -> decltype(
      std::forward<EventStream>(es).put_events(std::forward<EventRange>(er))
   ) {
      return std::forward<EventStream>(es).put_events(std::forward<EventRange>(er));
   }
} put_events = {};
}

namespace concepts {
template<typename T>
concept event_stream = event<typename T::event_type>
   && requires (T const &tc) {
      { event_stream_id(tc) } -> concepts::identifier;
      { events(tc) } -> event_range;
      { events(tc, std::declval<event_stream_sequence_t<T>>(), std::declval<event_stream_sequence_t<T>>().next()) } -> event_range;
      requires std::same_as<
            event_t<decltype(events(tc, std::declval<event_stream_sequence_t<T>>(), std::declval<event_stream_sequence_t<T>>().next()))>,
            event_t<T>
      >;
};
}

template<concepts::event_stream EventStream>
using event_stream_head_t = std::tuple<event_stream_id_t<EventStream>, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>;

namespace details_ {
template<typename, typename, typename=void>
struct put_events_result_type_impl;

template<concepts::event_stream EventStream, concepts::event_range EventRange>
struct put_events_result_type_impl<EventStream, EventRange, std::void_t<std::invoke_result_t<decltype(put_events), EventStream &, EventRange>>> {
   using type = std::invoke_result_t<decltype(put_events), EventStream &&, EventRange &&>;
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

template<concepts::event_stream EventStream, concepts::sequenced Sequence>
using events_t = decltype(events(std::declval<EventStream>(), std::declval<Sequence>()));

namespace details_ {
template<typename Mutex, typename=void>
struct read_lock {
   using type = std::scoped_lock<Mutex>;
};

template<typename Mutex>
struct read_lock<Mutex, std::void_t<decltype(std::declval<Mutex>().lock_shared())>> {
   using type = std::shared_lock<Mutex>;
};
}

template<
   typename EventStream,
   concepts::event Event
>
class event_stream_base {
private:
   friend EventStream;

public:
   template<typename EventRange>
   requires concepts::event_range<EventRange>
      && std::same_as<event_stream_id_t<EventRange>, event_stream_id_t<Event>>
      && std::same_as<event_stream_sequence_t<EventRange>, event_stream_sequence_t<Event>>
      && std::same_as<event_stream_timestamp_t<EventRange>, event_stream_timestamp_t<Event>>
   constexpr auto put_events(EventRange &&events) {
      std::scoped_lock lock{derived().get_mutex()};
      derived().prepare_for_put();
      precommit const result{event_stream_id(derived_const()), derived().last_committed_sequence()};
      std::size_t num_events = 0;
      try {
         derived().persist(std::forward<EventRange>(events));
         return result.commit_success(derived_const().commit_id(), derived_const().commit_timestamp(), num_events);
      }
      catch (...) {
         derived().rollback_to(result.precommit_sequence());
         return result.commit_error(derived_const().commit_id(), derived_const().commit_timestamp(), std::current_exception());
      }
   }

private:
   constexpr void prepare_for_put() const noexcept {
   }

   template<typename Sequence>
   constexpr Sequence ending_sequence(Sequence const precommit_sequence, std::size_t const num_events) const noexcept {
      return Sequence{precommit_sequence.value() + static_cast<typename Sequence::value_type>(num_events)};
   }

   constexpr EventStream & derived() noexcept {
      return *static_cast<EventStream *>(this);
   }

   constexpr EventStream const & derived_const() const noexcept {
      return *static_cast<EventStream const *>(this);
   }
};

}