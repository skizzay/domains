#include <domains/utils/any.hpp>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <cstring>

using namespace domains;
using big_type = std::array<char, sizeof(void *) * 4>;
using small_type = int;

TEST_CASE("any type", "[any]") {
   small_type small = 22;
   big_type big;
   std::memset(big.data(), 0, big.size());

   SECTION("is empty for default constructor") {
      any target;
      REQUIRE(target.empty());
   }
   SECTION("returns nullptr when empty") {
      any target;
      REQUIRE(any_cast<std::string>(&target) == nullptr);
   }
   SECTION("returns nullptr when casted from big type to wrong type") {
      any target(std::string("hi"));
      REQUIRE(any_cast<std::vector<char>>(&target) == nullptr);
   }
   SECTION("will throw bad_any_cast if empty") {
      any target;
      REQUIRE_THROWS_AS(any_cast<std::string>(target), bad_any_cast);
   }
   SECTION("will throw bad_any_cast if casting to a different type") {
      any target(std::vector<int>{1, 2, 3});
      REQUIRE_THROWS_AS(any_cast<std::string>(target), bad_any_cast);
   }

   SECTION("is not empty for big parameterized constructor") {
      any target(big);
      REQUIRE(!target.empty());
   }
   SECTION("is empty after clearing") {
      any target(big);
      target.clear();
      REQUIRE(target.empty());
   }
   SECTION("can be casted into big populated type") {
      any target(big);
      REQUIRE(any_cast<big_type>(&target) != nullptr);
      REQUIRE(*any_cast<big_type>(&target) == big);
   }
   SECTION("can be copied into another big any type") {
      any source(big);
      any target(source);
      REQUIRE(*any_cast<big_type>(&target) == big);
   }
   SECTION("can be directly casted into big populated type") {
      any target(big);
      REQUIRE(any_cast<big_type>(target) == big);
   }

   SECTION("is not empty for small parameterized constructor") {
      any target(small);
      REQUIRE(!target.empty());
   }
   SECTION("is empty after clearing") {
      any target(small);
      target.clear();
      REQUIRE(target.empty());
   }
   SECTION("can be casted into small populated type") {
      any target(small);
      REQUIRE(any_cast<small_type>(&target) != nullptr);
      REQUIRE(*any_cast<small_type>(&target) == small);
   }
   SECTION("can be copied into another small any type") {
      any source(small);
      any target(source);
      REQUIRE(*any_cast<small_type>(&target) == small);
   }
   SECTION("can be directly casted into small populated type") {
      any target(small);
      REQUIRE(any_cast<small_type>(target) == small);
   }
}
