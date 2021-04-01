#pragma once

#include <skizzay/domains/event.h>
#include <skizzay/domains/commit.h>
#include <skizzay/domains/concurrency_collision.h>
#include <functional>
#include <limits>
#include <mutex>
#include <ranges>

namespace skizzay::domains {
template<concepts::sequenced Sequence>
inline constexpr Sequence max_ending_exclusive_version{std::numeric_limits<typename Sequence::value_type>::max()};

inline namespace events_details_ {
inline constexpr struct events_function_ final {
   template <typename EventStream>
   constexpr auto operator()(EventStream const &e) const noexcept(
      std::is_nothrow_invocable_v<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>>
   ) -> std::invoke_result_t<events_function_, EventStream const &, event_stream_sequence_t<EventStream>, event_stream_sequence_t<EventStream>> {
      return std::invoke(*this, static_cast<EventStream const &>(e), event_stream_sequence_t<EventStream>{}, max_ending_exclusive_version<event_stream_sequence_t<EventStream>>);
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
         derived().persist(as_validated_events(std::forward<EventRange>(events), result, num_events));
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

   template<typename EventRange, typename Precommit>
   constexpr auto as_validated_events(EventRange &&events, Precommit const &p, std::size_t &num_events) const {
      if (std::ranges::empty(events)) {
         throw std::range_error{"Empty event range provided"};
      }
      else {
         auto validate_event = [last_sequence=p.precommit_sequence(), expected_event_stream_id=p.event_stream_id()](auto &&event) mutable {
            if (event_stream_id(event) != expected_event_stream_id) {
               throw std::invalid_argument{"Event stream id does match the event's id"};
            }
            else if (event_stream_sequence(event) != last_sequence.next()) {
               throw concurrency_collision{"Expected event stream sequence was not found"};
            }
            else {
               last_sequence = last_sequence.next();
               return event;
            }
         };
         auto increment_event_count = [&num_events](auto &&event) mutable noexcept {
            ++num_events;
            return event;
         };
         return std::forward<EventRange>(events)
            | std::views::transform(validate_event)
            | std::views::transform(increment_event_count);
      }
   }

   constexpr EventStream & derived() noexcept {
      return *static_cast<EventStream *>(this);
   }

   constexpr EventStream const & derived_const() const noexcept {
      return *static_cast<EventStream const *>(this);
   }
};

}