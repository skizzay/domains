#pragma once

#include <optional>
#include <vector>
#include <skizzay/domains/event_source/concepts.h
#include <skizzay/domains/event_source/event.h>

namespace skizzay::domains::event_source {

namespace concepts {
template <typename T, typename E = std::error_code>
concept commit = requires(T const &t) {
   { t.is_error() }
   ->std::same_as<bool>;
   { t.event_stream_id() }
   ->identifier;
   { t.event_starting_sequence() }
   ->sequenced;
   { t.events() }
   ->event_range;
   { t.error() }
   ->std::same_as<std::optional<E>>;
};
} // namespace concepts
namespace details_ {
template <typename T>
constexpr T throw_error(std::error_code e) {
   throw std::system_error(std::move(e));
}

template <typename T>
constexpr T throw_error(std::exception const &e) {
   throw e;
}
} // namespace details_

template <concepts::identifier CommitIdType, concepts::dispatchable_event EventType, typename ErrorType>
struct basic_commit {
   using commit_id_type = CommitIdType;
   using event_stream_id_type = std::remove_cvref_t<decltype(
      skizzay::domains::event_source::event_stream_id(std::declval<EventType>()))>;
   using event_stream_sequence_type = std::remove_cvref_t<decltype(
      skizzay::domains::event_source::event_stream_sequence(std::declval<EventType>()))>;
   using commit_timestamp_type = std::remove_cvref_t<decltype(
      skizzay::domains::event_source::event_stream_timestamp(std::declval<EventType>()))>;
   using error_type = ErrorType;

   template <std::input_iterator I, std::sentinel_for<I> S>
   constexpr explicit basic_commit(commit_id_type commit_id, I begin, S end)
      : value_{std::in_place_type<commit_context>,
               skizzay::domains::event_source::event_stream_timestamp(*begin),
               skizzay::domains::event_source::event_stream_id(*begin),
               skizzay::domains::event_source::event_stream_sequence(*begin),
               {begin, end}} {
   }

   template <typename R>
   requires concepts::dispatchable_event_range<R> &&
      std::ranges::input_range<R> constexpr explicit basic_commit(
         commit_id_type commit_id, R events)
      : basic_commit{std::move(commit_id), std::ranges::begin(events), std::ranges::end(events)} {
   }

   constexpr basic_commit(error_type e)
      : value_{std::in_place_type<error_type>, std::move(e)}
   {
   }

   constexpr bool is_error() const noexcept {
      return 1 == value_.index();
   }

   constexpr commit_id_type commit_id() const {
      return context().commit_id;
   }

   constexpr commit_timestamp_type commit_timestamp() const {
      return context().commit_timestamp;
   }

   constexpr event_stream_id_type event_stream_id() const {
      return context().event_stream_id;
   }

   constexpr event_stream_sequence_type event_stream_starting_sequence() const {
      return context().event_stream_starting_sequence;
   }

   constexpr std::pmr::vector<event_type> const & events() const {
      return context().events;
   }

   constexpr std::optional<ErrorType> error() const noexcept {
      return is_error() ? std::make_optional(std::get<1>(value_)) : std::nullopt;
   }

private:
   struct commit_context final {
      commit_id_type commit_id;
      commit_timestamp_type commit_timestamp;
      event_stream_id_type event_stream_id;
      event_stream_sequence_type event_stream_starting_sequence;
      std::pmr::vector<EventType> events;
   };

   constexpr commit_context const & context() const {
      return is_error() ? details_::throw_error<commit_context>(std::get<1>(value_))
                        : std::get<0>(value_);
   }

   std::variant<commit_context, error_type> value_;
};
}