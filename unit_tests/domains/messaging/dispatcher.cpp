#include "domains/messaging/dispatcher.hpp"
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <algorithm>
#include <experimental/string_view>
#include <iostream>

using namespace domains;

struct A {
   virtual std::string name() const = 0;
};

struct B final : A {
   virtual std::string name() const override {
      return "B";
   }
};

struct C : A {
   virtual std::string name() const override final {
      return "C";
   }
};

struct D final {};

TEST_CASE("Single dispatcher", "[dispatcher]") {
   auto target = make_single_dispatcher(
       [](std::experimental::string_view s) { return s.size(); }, [](int i) { return i; },
       [](A const &a) { return a.name(); }, [](C const &c) { return c.name(); });

   SECTION("dispatcher knows how to dispatch concrete types") {
      REQUIRE(target("help") == 4);
      REQUIRE(target(25) == 25);
   }

   SECTION("dispatcher knows how to fallback to base types") {
      REQUIRE(target(B()) == "B");
   }

   SECTION("dispatcher knows how to dispatch child types") {
      REQUIRE(target(C()) == "C");
   }
}

TEST_CASE("Multi dispatcher", "[dispatcher]") {
   size_t sv_invocations = 0;
   size_t a_invocations = 0;
   size_t b_invocations = 0;
   size_t c_invocations = 0;
   size_t d_invocations = 0;
   size_t int_invocations = 0;
   multi_dispatcher<std::string, B, C, D> target;
   target.add_handlers([&](std::experimental::string_view) { ++sv_invocations; },
                       [&](A const &) { ++a_invocations; }, [&](C const &) { ++c_invocations; });
   target.add_handler([&](int) { ++int_invocations; });

   SECTION("can route to handler on convertible type") {
      dispatch(target, std::string{"help"});
      REQUIRE(sv_invocations == 1);
      REQUIRE(a_invocations == 0);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 0);
      REQUIRE(d_invocations == 0);
      REQUIRE(int_invocations == 0);
   }

   SECTION("can route to handler on base type") {
      dispatch(target, B{});
      REQUIRE(sv_invocations == 0);
      REQUIRE(a_invocations == 1);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 0);
      REQUIRE(d_invocations == 0);
      REQUIRE(int_invocations == 0);
   }

   SECTION("can route to handler on child type") {
      dispatch(target, C{});
      REQUIRE(sv_invocations == 0);
      REQUIRE(a_invocations == 0);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 1);
      REQUIRE(d_invocations == 0);
      REQUIRE(int_invocations == 0);
   }

   SECTION("will ignore unhandle types") {
      REQUIRE_NOTHROW(dispatch(target, D{}));
      REQUIRE(sv_invocations == 0);
      REQUIRE(a_invocations == 0);
      REQUIRE(b_invocations == 0);
      REQUIRE(c_invocations == 0);
      REQUIRE(d_invocations == 0);
      REQUIRE(int_invocations == 0);
   }
}
