#include <skizzay/domains/event_stream.h>
#include <skizzay/domains/sequence.h>
#include <chrono>
#include <vector>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif


using namespace skizzay::domains;

namespace {

using id_type = std::string;
using sequence_type = sequence<struct test_sequence, std::uint32_t>;
using timestamp_type = std::chrono::high_resolution_clock::time_point;
using commit_type = basic_commit<id_type, id_type, sequence_type, timestamp_type>;
struct test_event : tagged_event<struct test, std::string, sequence_type, timestamp_type> {
   using tagged_event<struct test, std::string, sequence_type, timestamp_type>::tagged_event;
};

template<concepts::event Event>
struct event_stream_fake {
   using event_type = Event;

   id_type event_stream_id() const {
      return "event_stream_id";
   }

   template<concepts::event_range EventRange>
   commit_type put_events(EventRange const &events) {
      if (std::ranges::empty(events)) {
         return {
            "commit_id",
            event_stream_id(),
            std::chrono::high_resolution_clock::now(),
            std::make_exception_ptr(std::invalid_argument{"Empty event range"})
         };
      }
      else {
         return {
            "commit_id",
            event_stream_id(),
            std::chrono::high_resolution_clock::now(),
            event_stream_sequence(*std::ranges::begin(events)),
            event_stream_sequence(*(std::ranges::end(events) - 1))
         };
      }
   }

   std::ranges::empty_view<event_type> events(
      [[maybe_unused]] event_stream_sequence_t<event_type> begin_exclusive,
      [[maybe_unused]] event_stream_sequence_t<event_type> end_inclusive
   ) const {
      return std::views::empty<event_type>;
   }
};

}

TEST_CASE("Event Stream", "[event_source, event_stream]") {
   event_stream_fake<test_event> target;
   sequence_type begin{1U};
   sequence_type end{2U};

   SECTION("event_stream matches on events and event_stream_id") {
      REQUIRE(concepts::identifier<decltype(target.event_stream_id())>);
      REQUIRE((concepts::event_range<decltype(events(target, begin, end))>));
      REQUIRE(concepts::event_stream<event_stream_fake<test_event>>);
   }

   SECTION("event_range_writer matches on put_events") {
      REQUIRE(concepts::event_stream<event_stream_fake<test_event>>);
      REQUIRE(concepts::event_range<std::vector<test_event>>);
      REQUIRE((std::same_as<event_stream_id_t<std::vector<test_event>>, event_stream_id_t<event_stream_fake<test_event>>>));
      REQUIRE((std::same_as<event_stream_sequence_t<std::vector<test_event>>, event_stream_sequence_t<event_stream_fake<test_event>>>));
      REQUIRE((std::same_as<event_stream_timestamp_t<std::vector<test_event>>, event_stream_timestamp_t<event_stream_fake<test_event>>>));
      REQUIRE((
         std::same_as<commit_t<event_stream_fake<test_event>, std::vector<test_event>>, commit_type>
      ));
      REQUIRE((std::same_as<
         event_stream_id_t<std::vector<test_event>>,
         event_stream_id_t<commit_t<event_stream_fake<test_event>, std::vector<test_event>>>
      >));
      REQUIRE((std::same_as<
         event_stream_timestamp_t<std::vector<test_event>>,
         decltype(std::declval<commit_t<event_stream_fake<test_event>, std::vector<test_event>>>().commit_timestamp())
      >));
      REQUIRE((std::same_as<
         event_stream_sequence_t<std::vector<test_event>>,
         decltype(std::declval<commit_t<event_stream_fake<test_event>, std::vector<test_event>>>().event_stream_starting_sequence())
      >));
   }
}