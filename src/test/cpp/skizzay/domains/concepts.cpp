#include <skizzay/domains/concepts.h>
#include <string>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains::concepts;

namespace {
struct not_totally_ordered {
    bool operator<=>(not_totally_ordered const &) const = delete;
    int value() const;
    not_totally_ordered next() const;
    not_totally_ordered previous() const;
};

struct not_copyable {
    not_copyable(not_copyable const &) = delete;
    auto operator<=>(not_copyable const &) const = default;
    int value() const;
    not_copyable next() const;
    not_copyable previous() const;
};

struct not_a_value_object {
    auto operator<=>(not_a_value_object const &) const = default;
    not_a_value_object next() const;
    not_a_value_object previous() const;
};

struct missing_next {
    auto operator<=>(missing_next const &) const = default;
    int value() const;
    missing_next previous() const;
};

struct missing_previous {
    auto operator<=>(missing_previous const &) const = default;
    int value() const;
    missing_previous next() const;
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


TEST_CASE("Sequenced", "[event_source, concepts, sequenced]") {
   SECTION("Not totally ordered is not sequenced") {
      REQUIRE(!sequenced<not_totally_ordered>);
   }

   SECTION("Not regular is not sequenced") {
      REQUIRE(!sequenced<not_copyable>);
   }
   
   SECTION("Not a value object is not sequenced") {
      REQUIRE(!sequenced<not_a_value_object>);
   }
   
   SECTION("Missing next is not sequenced") {
      REQUIRE(!sequenced<missing_next>);
   }
   
   SECTION("Missing previous is not sequenced") {
      REQUIRE(!sequenced<missing_previous>);
   }
}