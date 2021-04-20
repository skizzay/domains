#pragma once

#include <memory_resource>
#include <system_error>
#include <optional>
#include <vector>
#include <skizzay/domains/concepts.h>
#include <skizzay/domains/event.h>
#include <skizzay/domains/event_stream_head.h>
#include <skizzay/domains/validated_event_range.h>

namespace skizzay::domains {

namespace concepts {
template <typename T>
concept commit = requires(T const &t) {
   typename T::error_type;
   { t.is_error() } -> std::same_as<bool>;
   { t.commit_id() } -> concepts::identifier;
   { t.commit_timestamp() } -> concepts::timestamp;
   { t.event_stream_id() }->concepts::identifier;
   { t.event_stream_starting_sequence() }->concepts::sequenced;
   { t.event_stream_ending_sequence() }->concepts::sequenced;
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


template <
   concepts::identifier CommitIdType,
   concepts::identifier EventStreamIdType,
   concepts::sequenced Sequence,
   concepts::timestamp Timestamp,
   typename ErrorType=std::exception_ptr
>
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


template<concepts::identifier CommitId, concepts::event_stream_head EventStreamHead, typename ErrorType=std::exception_ptr>
constexpr basic_commit<CommitId, event_stream_id_t<EventStreamHead>, event_stream_sequence_t<EventStreamHead>, event_stream_timestamp_t<EventStreamHead>, ErrorType>
commit_success(CommitId commit_id, EventStreamHead const &before, EventStreamHead const &after, [[maybe_unused]] ErrorType error={}) {
   return basic_commit{std::move(commit_id), event_stream_id(before), event_stream_timestamp(after), event_stream_sequence(before).next(), event_stream_sequence(after)};
}


template<concepts::identifier CommitId, concepts::event_stream_head EventStreamHead, typename ErrorType>
constexpr basic_commit<CommitId, event_stream_id_t<EventStreamHead>, event_stream_sequence_t<EventStreamHead>, event_stream_timestamp_t<EventStreamHead>, ErrorType>
commit_error(CommitId commit_id, EventStreamHead const &before, event_stream_timestamp_t<EventStreamHead> commit_timestamp, ErrorType error) {
   return basic_commit<CommitId, event_stream_id_t<EventStreamHead>, event_stream_sequence_t<EventStreamHead>, event_stream_timestamp_t<EventStreamHead>, ErrorType>{
      std::move(commit_id),
      event_stream_id(before),
      std::move(commit_timestamp),
      std::move(error)
   };
}


template<
   concepts::identifier EventStreamId,
   concepts::sequenced EventStreamSequence
>
struct precommit final {
   constexpr precommit(EventStreamId event_stream_id, EventStreamSequence precommit_sequence) noexcept :
      event_stream_id_{std::move(event_stream_id)},
      precommit_sequence_{std::move(precommit_sequence)}
   {
   }

   constexpr EventStreamId event_stream_id() const noexcept {
      return event_stream_id_;
   }

   constexpr EventStreamSequence precommit_sequence() const noexcept {
      return precommit_sequence_;
   }

   template<concepts::event_range EventRange>
   constexpr auto validate(EventRange &events) const {
      if (std::ranges::empty(events)) {
         throw std::range_error{"Range is empty"};
      }
      else {
         return events | ensure_event_is_sequenced(precommit_sequence_.next()) | ensure_event_id_matches(event_stream_id_);
      }
   }

   template<
      concepts::identifier CommitId,
      concepts::timestamp CommitTimestamp,
      typename Error=std::exception_ptr
   >
   constexpr auto commit_success(CommitId commit_id, CommitTimestamp commit_timestamp, EventStreamSequence::value_type const num_events, [[maybe_unused]] Error e={}) const noexcept {
      return basic_commit<CommitId, EventStreamId, EventStreamSequence, CommitTimestamp, Error> {
         std::move(commit_id),
         event_stream_id_,
         std::move(commit_timestamp),
         precommit_sequence_.next(),
         EventStreamSequence{precommit_sequence_.value() + num_events}
      };
   }

   template<
      concepts::identifier CommitId,
      concepts::timestamp CommitTimestamp,
      typename Error
   >
   constexpr auto commit_error(CommitId commit_id, CommitTimestamp commit_timestamp, Error e) const noexcept {
      return basic_commit<CommitId, EventStreamId, EventStreamSequence, CommitTimestamp, Error> {
         std::move(commit_id),
         event_stream_id_,
         std::move(commit_timestamp),
         std::move(e)
      };
   }

private:
   EventStreamId event_stream_id_;
   EventStreamSequence precommit_sequence_;
};

}