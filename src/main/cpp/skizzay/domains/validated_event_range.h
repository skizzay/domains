#pragma once

#include <skizzay/domains/event.h>
#include <iterator>
#include <concepts>
#include <ranges>
#include <sstream>

namespace skizzay::domains {

inline constexpr auto pass_thru = []<typename F>(F &&f) {
   return std::ranges::views::transform([f = std::forward<F>(f)]<typename T>(T &&t) {
      std::invoke(f, static_cast<T const &>(t));
      return std::forward<T>(t);
   });
};

inline constexpr auto ensure_nonempty = []<std::ranges::range Range>(Range &r) {
      if (std::ranges::empty(std::forward<Range>(r))) {
         throw std::range_error{"Range is empty"};
      }
      else {
         return std::forward<Range>(r);
      }
   };


inline constexpr auto ensure_event_is_sequenced = []<concepts::sequenced EventStreamSequence>(EventStreamSequence expected_sequence) {
      return std::ranges::views::transform([expected_sequence=std::move(expected_sequence)]<concepts::event Event>(Event &&e) mutable {
         if (event_stream_sequence(e) != expected_sequence) {
            std::ostringstream oss;
            oss << "Expected to find sequence {"
               << expected_sequence.value()
               << "} but instead found sequence {"
               << event_stream_sequence(e).value()
               << "}";
            throw std::invalid_argument{oss.str()};
         }
         else {
            expected_sequence = expected_sequence.next();
         }
         return std::forward<Event>(e);
      });
   };


inline constexpr auto ensure_event_id_matches = []<concepts::identifier EventStreamId>(EventStreamId expected_id) {
      return pass_thru([expected_id=std::move(expected_id)]<concepts::event Event>(Event const &e) {
         if (event_stream_id(e) != expected_id) {
            std::ostringstream oss;
            oss << "Expected to find id {"
               << expected_id
               << "} but instead found id {"
               << event_stream_id(e)
               << "}";
            throw std::invalid_argument{oss.str()};
         }
      });
   };

template<concepts::event_stream_head EventStreamHead, concepts::event_range EventRange>
inline constexpr auto validate_commit_range(EventStreamHead const &esh, EventRange &er) {
   if (std::ranges::empty(er)) {
      throw std::range_error{"Range is empty"};
   }
   else {
      return er | ensure_event_is_sequenced(event_stream_sequence(esh).next()) | ensure_event_id_matches(event_stream_id(esh));
   }
}

}