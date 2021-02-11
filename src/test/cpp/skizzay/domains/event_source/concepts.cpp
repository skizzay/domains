#include <skizzay/domains/event_source/concepts.h>
#include <string>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains::event_source::concepts;

namespace {
struct not_totally_ordered {
    bool operator<=>(not_totally_ordered const &) const = delete;
};

struct not_copyable {
    not_copyable(not_copyable const &) = delete;
};
}

TEST_CASE("Identifier", "[event_source, concepts, identifier]") {
   SECTION("totally ordered and copyable are identifiers") {
      REQUIRE(identifier<int>);
      REQUIRE(identifier<std::string>);
   }

   SECTION("not totally ordered is not an identifier") {
      REQUIRE(!identifier<not_totally_ordered>);
   }

   SECTION("not copyable is not an identifier") {
      REQUIRE(!identifier<not_copyable>);
   }
}


TEST_CASE("Timestamp", "[event_source, concepts, timestamp]") {
   SECTION("std::time_point is a timestamp") {
      REQUIRE(timestamp<std::chrono::system_clock::time_point>);
   }

   SECTION("Not a std::time_point is not a timestamp") {
      REQUIRE(!timestamp<int>);
   }
}