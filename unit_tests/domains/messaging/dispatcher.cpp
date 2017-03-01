#include "domains/messaging/dispatcher.hpp"
#if __has_include(<catch/catch.hpp>)
#  include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#  include <catch.hpp>
#endif
#include <algorithm>
#include <iostream>
#include <experimental/string_view>

using namespace domains;

namespace {

struct A {
   virtual std::string name() const = 0;
};

struct B : A {
   virtual std::string name() const override { return "B"; }
};

struct C : A {
   virtual std::string name() const override { return "C"; }
};

}

TEST_CASE("Dispatcher handles messages") {
   auto target = make_dispatcher(
         [] (std::experimental::string_view s) { return s.size(); },
         [] (int i) { return i; },
         [] (A const &a) { return a.name(); },
         [] (C const &c) { return c.name(); }
      );

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
