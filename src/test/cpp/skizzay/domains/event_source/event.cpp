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

   friend inline std::string tag_invoke(skizzay::domains::tag_t<event_stream_id>, customized_test_event const &) {
      return "customized";
   }

   friend inline test_sequence tag_invoke(skizzay::domains::tag_t<event_stream_sequence>,
                                          customized_test_event const &cte) {
      return cte.custom_event_sequence.next();
   }

   friend inline std::chrono::steady_clock::time_point
   tag_invoke(skizzay::domains::tag_t<event_stream_timestamp>, customized_test_event const &cte) {
      return cte.custom_event_timestamp + std::chrono::seconds{1};
   }
};

struct custom_event_handler {
   bool handled_test_event = false;
   bool handled_custom_test_event = false;

   void handle(test_event const &) {
      handled_test_event = true;
   }

   void handle(customized_test_event const &) {
      handled_custom_test_event = true;
   }

   friend inline void tag_invoke(skizzay::domains::tag_t<dispatch_event>, custom_event_handler &h,
                                 concepts::event auto const &e) {
      h.handle(e);
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

   SECTION("An event is dispatchable") {
      REQUIRE(concepts::dispatchable_event<test_event>);
      REQUIRE(concepts::dispatchable_event<customized_test_event const &>);
      REQUIRE(concepts::dispatchable_event<customized_test_event const *>);
      REQUIRE(concepts::dispatchable_event<test_event *>);
      REQUIRE(concepts::dispatchable_event<std::unique_ptr<test_event> *>);
      REQUIRE((concepts::dispatchable_event<variant_event<test_event, customized_test_event>>));
   }

   SECTION("Dispatching an event resolves handler") {
      bool handled = false;
      skizzay::domains::event_source::dispatch_event(
         [&handled](test_event const &) noexcept { handled = true; }, te);
      REQUIRE(handled);
   }


   SECTION("Dispatching a pointer to an event resolves handler") {
      bool handled = false;
      skizzay::domains::event_source::dispatch_event(
         [&handled](test_event const &) noexcept { handled = true; }, &te);
      REQUIRE(handled);
   }

   SECTION("CPO for event handler for dispatching an event resolves handler") {
      custom_event_handler h;
      skizzay::domains::event_source::dispatch_event(h, cte);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION("CPO for event handler for dispatching a pointer to an event resolves handler") {
      custom_event_handler h;
      skizzay::domains::event_source::dispatch_event(h, &cte);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION("Dispatching a variant of an event resolves handler") {
      bool handled = false;
      std::variant<test_event, customized_test_event> const v{cte};
      skizzay::domains::event_source::dispatch_event(
         [&handled](concepts::event auto const &) noexcept { handled = true; }, v);
      REQUIRE(handled);
   }

   SECTION("Dispatching a pointer to a variant of an event resolves handler") {
      bool handled = false;
      std::variant<test_event, customized_test_event> const v{cte};
      skizzay::domains::event_source::dispatch_event(
         [&handled](concepts::event auto const &) noexcept { handled = true; }, &v);
      REQUIRE(handled);
   }

   SECTION("Dispatching a variant of event pointers resolves handler") {
      bool handled = false;
      std::variant<test_event *, customized_test_event const *> const v{&cte};
      skizzay::domains::event_source::dispatch_event(
         [&handled](concepts::event auto const &) noexcept { handled = true; }, v);
      REQUIRE(handled);
   }

   SECTION("CPO for event handler for dispatching a variant of an event resolves handler") {
      custom_event_handler h;
      std::variant<test_event, customized_test_event> const v{te};
      skizzay::domains::event_source::dispatch_event(h, v);
      REQUIRE(h.handled_test_event);
   }

   SECTION("CPO for event handler for dispatching a variant of event pointers resolves handler") {
      custom_event_handler h;
      std::variant<test_event const *, customized_test_event const *> const v{&cte};
      skizzay::domains::event_source::dispatch_event(h, v);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION(
      "CPO for event handler for dispatching a pointer to a variant of an event resolves handler") {
      custom_event_handler h;
      std::variant<test_event, customized_test_event> const v{cte};
      skizzay::domains::event_source::dispatch_event(h, &v);
      REQUIRE(h.handled_custom_test_event);
   }

   SECTION(
      "CPO for event handler for dispatching a pointer to a variant of event pointers resolves handler") {
      custom_event_handler h;
      std::variant<test_event const *, customized_test_event const *> const v{&te};
      skizzay::domains::event_source::dispatch_event(h, &v);
      REQUIRE(h.handled_test_event);
   }
}