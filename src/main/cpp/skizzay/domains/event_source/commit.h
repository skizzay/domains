#pragma once

#include <memory_resource>
#include <system_error>
#include <optional>
#include <vector>
#include <skizzay/domains/event_source/concepts.h>
#include <skizzay/domains/event_source/event.h>

namespace skizzay::domains::event_source {

namespace concepts {
template <typename T>
concept commit = requires(T const &t) {
   typename T::error_type;
   { t.is_error() } -> std::same_as<bool>;
   { t.commit_id() } -> identifier;
   { t.commit_timestamp() } -> timestamp;
   { t.event_stream_id() }->identifier;
   { t.event_stream_starting_sequence() }->sequenced;
   { t.events() }->event_range;
   { t.error() }->std::same_as<std::optional<typename T::error_type>>;
};
} // namespace concepts


namespace details_ {
template <typename T>
constexpr T const & throw_error(std::error_code e) {
   throw std::system_error(std::move(e));
}

template <typename T>
constexpr T const & throw_error(std::exception_ptr e) {
   std::rethrow_exception(e);
}
} // namespace details_


template <concepts::identifier CommitIdType, concepts::event EventType, typename ErrorType>
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
   constexpr explicit basic_commit(commit_id_type commit_id, I begin, S end, std::pmr::polymorphic_allocator<EventType> allocator={})
      : commit_id_{std::move(commit_id)},
         commit_timestamp_{skizzay::domains::event_source::event_stream_timestamp(*begin)},
         value_{std::in_place_type<std::pmr::vector<EventType>>, begin, end, std::move(allocator)} {
   }

   template <typename R>
   requires concepts::event_range<R> &&
      std::ranges::input_range<R> constexpr explicit basic_commit(
         commit_id_type commit_id, R events, std::pmr::polymorphic_allocator<EventType> allocator={})
      : basic_commit{std::move(commit_id), std::ranges::begin(events), std::ranges::end(events), std::move(allocator)} {
   }

   constexpr basic_commit(commit_id_type commit_id, commit_timestamp_type commit_timestamp, error_type e)
      : commit_id_{std::move(commit_id)},
         commit_timestamp_{std::move(commit_timestamp)},
         value_{std::in_place_type<error_type>, std::move(e)}
   {
   }

   constexpr bool is_error() const noexcept {
      return 1 == value_.index();
   }

   constexpr commit_id_type commit_id() const {
      return commit_id_;
   }

   constexpr commit_timestamp_type commit_timestamp() const {
      return commit_timestamp_;
   }

   constexpr event_stream_id_type event_stream_id() const {
      return skizzay::domains::event_source::event_stream_id(events().front());
   }

   constexpr event_stream_sequence_type event_stream_starting_sequence() const {
      return skizzay::domains::event_source::event_stream_sequence(events().front());
   }

   constexpr std::pmr::vector<EventType> const & events() const {
      return is_error() ? details_::throw_error<std::pmr::vector<EventType>>(std::get<1>(value_))
                        : std::get<0>(value_);
   }

   constexpr std::optional<ErrorType> error() const noexcept {
      return is_error() ? std::make_optional(std::get<1>(value_)) : std::nullopt;
   }

private:
   commit_id_type commit_id_;
   commit_timestamp_type commit_timestamp_;
   std::variant<std::pmr::vector<EventType>, error_type> value_;
};
}