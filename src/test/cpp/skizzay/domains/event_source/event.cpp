#include <skizzay/domains/event_source/event.h>
#include <skizzay/domains/event_source/sequence.h>
#include <string>
#include <chrono>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains::event_source;

namespace {
using test_sequence = sequence<struct test, std::size_t>;
using test_event =
   tagged_event<struct test, std::string, test_sequence, std::chrono::steady_clock::time_point>;

struct customized_test_event {
   std::string custom_event_stream_id;
   test_sequence custom_event_sequence;
   std::chrono::steady_clock::time_point custom_event_timestamp;

   friend inline std::string tag_invoke(tag_t<event_stream_id>, customized_test_event const &) {
      return "customized";
   }

   friend inline test_sequence tag_invoke(tag_t<event_stream_sequence>,
                                          customized_test_event const &cte) {
      return cte.custom_event_sequence.next();
   }

   friend inline std::chrono::steady_clock::time_point
   tag_invoke(tag_t<event_stream_timestamp>, customized_test_event const &cte) {
      return cte.custom_event_timestamp + std::chrono::seconds{1};
   }
};
} // namespace

TEST_CASE("Event", "[event_source, event_stream_id]") {
   std::string const expected_stream_id = "test_event_stream_id";
   test_sequence const expected_sequence{12};
   std::chrono::steady_clock::time_point const expected_timestamp = std::chrono::steady_clock::now();
   test_event const te{expected_stream_id, expected_sequence, expected_timestamp};
   customized_test_event const cte{expected_stream_id, expected_sequence, expected_timestamp};

   SECTION("An event should be able to capture event stream id") {
      REQUIRE(skizzay::domains::event_source::event_stream_id(te) == expected_stream_id);
   }

   SECTION("Customization on tag_invocation for event stream id") {
      REQUIRE(skizzay::domains::event_source::event_stream_id(cte) == "customized");
   }

   SECTION("An event should be able to capture event stream sequence") {
      REQUIRE(skizzay::domains::event_source::event_stream_sequence(te) == expected_sequence);
   }

   SECTION("Customization on tag_invocation for event stream sequence") {
      REQUIRE(skizzay::domains::event_source::event_stream_sequence(cte) ==
              expected_sequence.next());
   }

   SECTION("An event should be able to capture event stream timestamp") {
      REQUIRE(skizzay::domains::event_source::event_stream_timestamp(te) == expected_timestamp);
   }

   SECTION("Customization on tag_invocation for event stream timestamp") {
      REQUIRE(skizzay::domains::event_source::event_stream_timestamp(cte) ==
              (expected_timestamp + std::chrono::seconds{1}));
   }
}