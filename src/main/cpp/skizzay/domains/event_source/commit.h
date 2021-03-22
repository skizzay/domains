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
   { t.event_stream_ending_sequence() }->sequenced;
   { t.error() }->std::same_as<std::optional<typename T::error_type>>;
   requires std::same_as<decltype(t.event_stream_starting_sequence()), decltype(t.event_stream_ending_sequence())>;
};
} // namespace concepts


namespace details_ {
template <typename Sequence>
constexpr std::tuple<Sequence, Sequence> sequence_range(std::error_code const &e) {
   throw std::system_error(std::move(e));
}

template <typename Sequence>
constexpr std::tuple<Sequence, Sequence> sequence_range(std::exception_ptr const &e) {
   std::rethrow_exception(e);
}

template <typename Sequence>
constexpr std::tuple<Sequence, Sequence> sequence_range(std::tuple<Sequence, Sequence> const &range) {
   return range;
}
} // namespace details_


template <concepts::identifier CommitIdType, concepts::identifier EventStreamIdType, concepts::sequenced Sequence, concepts::timestamp Timestamp, typename ErrorType=std::exception_ptr>
struct basic_commit {
   using commit_id_type = CommitIdType;
   using event_stream_id_type = EventStreamIdType;
   using event_stream_sequence_type = Sequence;
   using commit_timestamp_type = Timestamp;
   using error_type = ErrorType;

   constexpr explicit basic_commit(
      commit_id_type commit_id,
      event_stream_id_type event_stream_id,
      Timestamp commit_timestamp,
      Sequence start,
      Sequence end
   ) noexcept :
      commit_id_{std::move(commit_id)},
      event_stream_id_{std::move(event_stream_id)},
      commit_timestamp_{std::move(commit_timestamp)},
      value_{std::in_place_index<0>, start, end}
   {
   }

   constexpr explicit basic_commit(
      commit_id_type commit_id,
      event_stream_id_type event_stream_id,
      Timestamp commit_timestamp,
      error_type error
   ) noexcept :
      commit_id_{std::move(commit_id)},
      event_stream_id_{std::move(event_stream_id)},
      commit_timestamp_{std::move(commit_timestamp)},
      value_{std::in_place_index<1>, std::move(error)}
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
      return event_stream_id_;
   }

   constexpr event_stream_sequence_type event_stream_starting_sequence() const {
      return std::get<0>(sequence_range());
   }

   constexpr event_stream_sequence_type event_stream_ending_sequence() const {
      return std::get<1>(sequence_range());
   }

   constexpr std::optional<ErrorType> error() const noexcept {
      return is_error() ? std::make_optional(std::get<1>(value_)) : std::nullopt;
   }

private:
   std::tuple<Sequence, Sequence> sequence_range() const {
      return std::visit([](auto const &value) { return details_::sequence_range<event_stream_sequence_type>(value); }, value_);
   }

   commit_id_type commit_id_;
   event_stream_id_type event_stream_id_;
   commit_timestamp_type commit_timestamp_;
   std::variant<std::tuple<Sequence, Sequence>, error_type> value_;
};
}