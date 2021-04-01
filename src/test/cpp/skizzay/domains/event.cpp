#include <skizzay/domains/event.h>
#include <skizzay/domains/sequence.h>
#include <string>
#include <chrono>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains;
using namespace std::chrono_literals;

namespace {
using test_sequence = sequence<struct test, std::size_t>;
struct test_event : tagged_event<struct test, std::string, test_sequence, std::chrono::steady_clock::time_point> {
   using tagged_event<struct test, std::string, test_sequence, std::chrono::steady_clock::time_point>::tagged_event;
};

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
      return cte.custom_event_timestamp + 1s;
   }

   friend inline void tag_invoke(
      tag_t<with_timestamp>,
      customized_test_event &cte,
      std::chrono::steady_clock::time_point timestamp
   ) {
      cte.custom_event_timestamp = std::move(timestamp);
   }
};

struct custom_event_handler {
   bool handled_test_event = false;
   bool handled_custom_test_event = false;

   friend inline void tag_invoke(tag_t<dispatch_event>, custom_event_handler &h,
                                 test_event const &) {
      h.handled_test_event = true;
   }

   friend inline void tag_invoke(tag_t<dispatch_event>, custom_event_handler &h,
                                 customized_test_event const &) {
      h.handled_custom_test_event = true;
   }
};
} // namespace

TEST_CASE("Event", "[event_source, event]") {
   std::string const expected_stream_id = "test_event_stream_id";
   test_sequence const expected_sequence{12};
   std::chrono::steady_clock::time_point const expected_timestamp = std::chrono::steady_clock::now();
   test_event const te{expected_stream_id, expected_sequence, expected_timestamp};
   customized_test_event const cte{expected_stream_id, expected_sequence, expected_timestamp};

   SECTION("An event should be able to capture event stream id") {
      REQUIRE(event_stream_id(te) == expected_stream_id);
   }

   SECTION("Customization on tag_invocation for event stream id") {
      REQUIRE(event_stream_id(cte) == "customized");
      customized_test_event const &ref = cte;
      REQUIRE(event_stream_id(ref) == "customized");
   }

   SECTION("A variant of events should be able to retrieve the event stream id") {
      std::variant<test_event, customized_test_event> v{te};
      REQUIRE(event_stream_id(te) == event_stream_id(v));
   }

   SECTION("An event should be able to capture event stream sequence") {
      REQUIRE(event_stream_sequence(te) == expected_sequence);
   }

   SECTION("Customization on tag_invocation for event stream sequence") {
      REQUIRE(event_stream_sequence(cte) ==
              expected_sequence.next());
      customized_test_event const &ref = cte;
      REQUIRE(event_stream_sequence(ref) ==
              expected_sequence.next());
   }

   SECTION("A variant of events should be able to retrieve the event stream sequence") {
      std::variant<test_event, customized_test_event> v{cte};
      REQUIRE(event_stream_sequence(cte) == event_stream_sequence(v));
   }

   SECTION("An event should be able to capture event stream timestamp") {
      REQUIRE(event_stream_timestamp(te) == expected_timestamp);
   }

   SECTION("Customization on tag_invocation for event stream timestamp") {
      REQUIRE(event_stream_timestamp(cte) ==
              (expected_timestamp + std::chrono::seconds{1}));
      customized_test_event const &ref = cte;
      REQUIRE(event_stream_timestamp(ref) ==
              (expected_timestamp + std::chrono::seconds{1}));
   }

   SECTION("A variant of events should be able to retrieve the event stream timestamp") {
      std::variant<test_event, customized_test_event> v{cte};
      REQUIRE(event_stream_timestamp(cte) == event_stream_timestamp(v));
   }

   SECTION("An event is dispatchable") {
      REQUIRE(concepts::event<test_event>);
      REQUIRE(concepts::event<customized_test_event const &>);
      REQUIRE(concepts::event<customized_test_event const *>);
      REQUIRE(concepts::event<test_event *>);
      REQUIRE(concepts::event<std::unique_ptr<test_event> *>);
      REQUIRE((concepts::event<std::variant<test_event, customized_test_event>>));
   }

   SECTION("Dispatching an event resolves handler") {
      bool handled = false;
      dispatch_event(
         [&handled](test_event const &) noexcept { handled = true; }, te);
      REQUIRE(handled);
   }


   SECTION("Dispatching a pointer to an event resolves handler") {
      bool handled = false;
      dispatch_event(
         [&handled](test_event const &) noexcept -> void { handled = true; }, &te);
      REQUIRE(handled);
   }

   SECTION("CPO for event handler for dispatching an event resolves handler") {
      custom_event_handler h;
      dispatch_event(h, cte);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION("CPO for event handler for dispatching a pointer to an event resolves handler") {
      custom_event_handler h;
      dispatch_event(h, &cte);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION("Dispatching a variant of an event resolves handler") {
      bool handled = false;
      std::variant<test_event, customized_test_event> const v{cte};
      dispatch_event(
         [&handled](concepts::event auto const &) noexcept { handled = true; }, v);
      REQUIRE(handled);
   }

   SECTION("Dispatching a pointer to a variant of an event resolves handler") {
      bool handled = false;
      std::variant<test_event, customized_test_event> const v{cte};
      dispatch_event(
         [&handled](concepts::event auto const &) noexcept { handled = true; }, &v);
      REQUIRE(handled);
   }

   SECTION("Dispatching a variant of event pointers resolves handler") {
      bool handled = false;
      std::variant<test_event *, customized_test_event const *> const v{&cte};
      dispatch_event(
         [&handled](concepts::event auto const &) noexcept { handled = true; }, v);
      REQUIRE(handled);
   }

   SECTION("CPO for event handler for dispatching a variant of an event resolves handler") {
      custom_event_handler h;
      std::variant<test_event, customized_test_event> const v{te};
      dispatch_event(h, v);
      REQUIRE(h.handled_test_event);
   }

   SECTION("CPO for event handler for dispatching a variant of event pointers resolves handler") {
      custom_event_handler h;
      std::variant<test_event const *, customized_test_event const *> const v{&cte};
      dispatch_event(h, v);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION(
      "CPO for event handler for dispatching a pointer to a variant of an event resolves handler") {
      custom_event_handler h;
      std::variant<test_event, customized_test_event> const v{cte};
      dispatch_event(h, &v);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION(
      "CPO for event handler for dispatching a pointer to a variant of event pointers resolves handler") {
      custom_event_handler h;
      std::variant<test_event const *, customized_test_event const *> const v{&te};
      dispatch_event(h, &v);
      REQUIRE(h.handled_test_event);
   }

   SECTION("Mutable event updates existing event stream timestamp value") {
      std::chrono::steady_clock::time_point const next_expected_timestamp = expected_timestamp + 1s;
      std::variant<test_event, customized_test_event> v{te};
      // Quick sanity check to ensure that we have an updated timestamp
      REQUIRE(event_stream_timestamp(v) < next_expected_timestamp);
      with_timestamp(v, next_expected_timestamp);
      REQUIRE(event_stream_timestamp(v) == next_expected_timestamp);
   }
}