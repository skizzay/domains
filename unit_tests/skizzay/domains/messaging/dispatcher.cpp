#include "domains/messaging/dispatcher.hpp"

#include "unit_tests/domains/messaging/base_fixture.hpp"
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <algorithm>
#include <iostream>

using namespace domains;

TEST_CASE("Single dispatcher", "[dispatcher]") {
   factory f;
   auto target = f.make_single_domain_dispatcher();

   SECTION("dispatcher knows how to dispatch concrete types") {
      REQUIRE(target("help") == 4);
      REQUIRE(target(25) == 25);
   }

   SECTION("dispatcher knows how to fallback to base types") {
      target(G());
      REQUIRE(f.a_count == 1);
   }

   SECTION("dispatcher knows how to dispatch child types") {
      target(E());
      REQUIRE(f.e_count == 1);
   }
}

TEST_CASE("Multi dispatcher", "[dispatcher]") {
   size_t sv_invocations = 0;
   size_t a_invocations = 0;
   size_t b_invocations = 0;
   size_t c_invocations = 0;
   size_t other_invocations = 0;
   multi_dispatcher<void(std::experimental::string_view), void(B const &), void(C const &)> target;
   target.add_handler(single_dispatcher(
       [&](std::experimental::string_view) { ++sv_invocations; },
       [&](A const &) { ++a_invocations; }, [&](C const &) { ++c_invocations; }));
   target.add_handler([&](auto const &) { ++other_invocations; });

   SECTION("can route to handler on same type") {
      target(std::experimental::string_view{"help"});
      REQUIRE(sv_invocations == 1);
      REQUIRE(a_invocations == 0);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 0);
      REQUIRE(other_invocations == 1);
   }

   SECTION("can route to handler on convertible type") {
      target("help");
      REQUIRE(sv_invocations == 1);
      REQUIRE(a_invocations == 0);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 0);
      REQUIRE(other_invocations == 1);
   }

   SECTION("can route to handler on base type") {
      target(B{});
      REQUIRE(sv_invocations == 0);
      REQUIRE(a_invocations == 1);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 0);
      REQUIRE(other_invocations == 1);
   }

   SECTION("can route to handler on child type") {
      target(C{});
      REQUIRE(sv_invocations == 0);
      REQUIRE(a_invocations == 0);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 1);
      REQUIRE(other_invocations == 1);
   }
}
