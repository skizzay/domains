#include <skizzay/domains/aggregate/entity.h>
#include <skizzay/domains/event_source/sequence.h>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <string>

using namespace skizzay::domains::aggregate;
using skizzay::domains::tag_t;

namespace {
using entity_id_type = std::string;
using entity_version_type = skizzay::domains::event_source::sequence<struct test, std::size_t>;

struct test_entity {
   entity_id_type entity_id_;
   entity_version_type entity_version_ = {};

   entity_id_type entity_id() const {
      return entity_id_;
   }

   entity_version_type entity_version() const {
      return entity_version_;
   }
};

struct tagged_entity_id {
   entity_id_type id;

   friend entity_id_type tag_invoke(tag_t<entity_id>, tagged_entity_id const &x) {
      return x.id;
   }
};

struct tagged_entity_version {
   entity_version_type version;

   friend entity_version_type tag_invoke(tag_t<entity_version>, tagged_entity_version const &x) {
      return x.version;
   }
};
}

TEST_CASE("entity_id", "[aggregate, entity]") {
   SECTION("Member function") {
      entity_id_type const expected{"entity_id"};
      test_entity target{expected};
      auto const actual = entity_id(target);

      SECTION("is invoked") {
         REQUIRE(actual == expected);
      }
   }

   SECTION("Tagged dispatch") {
      entity_id_type const expected{"entity_id"};
      tagged_entity_id target{expected};
      auto const actual = entity_id(target);

      SECTION("is invoked") {
         REQUIRE(actual == expected);
      }
   }
}

TEST_CASE("entity_version", "[aggregate, entity]") {
   SECTION("Member function") {
      entity_version_type const expected{55};
      test_entity target{"entity_id", expected};
      auto const actual = entity_version(target);

      SECTION("is invoked") {
         REQUIRE(actual == expected);
      }
   }

   SECTION("Tagged dispatch") {
      entity_version_type const expected{55};
      tagged_entity_version target{expected};
      auto const actual = entity_version(target);

      SECTION("is invoked") {
         REQUIRE(actual == expected);
      }
   }
}