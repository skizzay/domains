#include <skizzay/domains/aggregate/entity_store.h>
#include <skizzay/domains/sequence.h>
#include <string>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains::aggregate;
using skizzay::domains::sequence;

namespace {

using entity_id_type = std::string;
using entity_version_type = sequence<struct test, std::size_t>;

struct test_entity {
   entity_id_type id;
   entity_version_type version;
   int i = {};

   entity_id_type entity_id() const noexcept {
      return id;
   }

   entity_version_type entity_version() const noexcept {
      return version;
   }
};

auto create_test_entity = [](entity_id_type id) -> test_entity {
   return test_entity{std::move(id), entity_version_type{}, 23};
};

}

TEST_CASE("basic_entity_factory", "[entity_factory][aggregate]") {
   using target_type = basic_entity_factory<test_entity, int>;
   int expected_i = 25;
   entity_id_type expected_entity_id = "hello";
   target_type target{expected_i};

   SECTION("when the entity is created") {
      auto actual{get_entity(target, expected_entity_id)};

      SECTION("then the entity id matches the input value") {
         REQUIRE(entity_id(actual) == expected_entity_id);
      }

      SECTION("then the entity version is zero") {
         REQUIRE(entity_version_type{} == entity_version(actual));
      }

      SECTION("then the provided value was passed onto the resulting entity") {
         REQUIRE(expected_i == actual.i);
      }
   }
}

TEST_CASE("shared_ptr_entity_factory", "[entity_factory][aggregate]") {
   using target_type = shared_ptr_entity_factory<test_entity, decltype(create_test_entity)>;
   entity_id_type expected_entity_id = "beer";
   target_type target{create_test_entity};

   SECTION("when the entity is created") {
      auto actual{get_entity(target, expected_entity_id)};

      SECTION("then the entity id matches the input value") {
         REQUIRE(entity_id(actual) == expected_entity_id);
      }

      SECTION("then the entity version is zero") {
         REQUIRE(entity_version_type{} == entity_version(actual));
      }
   }
}

TEST_CASE("unique_ptr_entity_factory", "[entity_factory][aggregate]") {
   using target_type = unique_ptr_entity_factory<test_entity, decltype(create_test_entity)>;
   entity_id_type expected_entity_id = "whiskey";
   target_type target{create_test_entity};

   SECTION("when the entity is created") {
      auto actual{get_entity(target, expected_entity_id)};

      SECTION("then the entity id matches the input value") {
         REQUIRE(entity_id(actual) == expected_entity_id);
      }

      SECTION("then the entity version is zero") {
         REQUIRE(entity_version_type{} == entity_version(actual));
      }
   }
}