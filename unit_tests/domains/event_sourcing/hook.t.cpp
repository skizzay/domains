#include "skizzay/domains/event_sourcing/hook.h"
#include <catch.hpp>

using namespace skizzay::domains::event_sourcing;

struct fake_on_commit_attempt {
   bool return_result;
   int on_commit_attempt_input = 0;

   constexpr bool on_commit_attempt(int const input) noexcept {
      on_commit_attempt_input = input;
      return return_result;
   }
};

struct fake_on_commit {
   int on_commit_input = 0;

   constexpr void on_commit(int const input) noexcept {
      on_commit_input = input;
   }
};

struct fake_on_stream_deleted {
   int on_stream_deleted_input = 0;

   constexpr void on_stream_deleted(int const input) noexcept {
      on_stream_deleted_input = input;
   }
};


TEST_CASE( "hook::on_commit_attempt", "[unit][hook]" ) {
   SECTION( "returns true if no hooks present" ) {
      hook target{};

      REQUIRE(target.on_commit_attempt(1));
   }

   SECTION( "returns true if no hooks support call" ) {
      hook target{fake_on_commit{}};

      REQUIRE(target.on_commit_attempt(2));
   }

   SECTION( "returns true if hook returns true" ) {
      hook target{fake_on_commit_attempt{true}};

      REQUIRE(target.on_commit_attempt(3));
   }

   SECTION( "returns false if hook returns false" ) {
      hook target{fake_on_commit_attempt{false}};

      REQUIRE_FALSE(target.on_commit_attempt(4));
   }

   SECTION( "invokes compatible hooks" ) {
      int const expected = 5;
      hook target{fake_on_commit_attempt{true}};

      target.on_commit_attempt(expected);

      REQUIRE(expected == target.on_commit_attempt_input);
   }
}

TEST_CASE( "hook::on_commit", "[unit][hook]" ) {
   SECTION( "invokes nothing when incompatible" ) {
      int const expected = 4;
      hook target{fake_on_stream_deleted{}};

      target.on_commit(expected);

      // The fact that this compiles is sufficient
      REQUIRE(true);
   }

   SECTION( "invokes nothing when empty" ) {
      int const expected = 4;
      hook target{};

      target.on_commit(expected);

      // The fact that this compiles is sufficient
      REQUIRE(true);
   }

   SECTION( "invokes compatible hooks" ) {
      int const expected = 5;
      hook target{fake_on_commit{}};

      target.on_commit(expected);

      REQUIRE(expected == target.on_commit_input);
   }
}

TEST_CASE( "hook::on_stream_deleted", "[unit][hook]" ) {
   SECTION( "invokes nothing when incompatible" ) {
      int const expected = 4;
      hook target{fake_on_commit{}};

      target.on_stream_deleted(expected);

      // The fact that this compiles is sufficient
      REQUIRE(true);
   }

   SECTION( "invokes nothing when empty" ) {
      int const expected = 4;
      hook target{};

      target.on_stream_deleted(expected);

      // The fact that this compiles is sufficient
      REQUIRE(true);
   }

   SECTION( "invokes compatible hooks" ) {
      int const expected = 5;
      hook target{fake_on_stream_deleted{}};

      target.on_stream_deleted(expected);

      REQUIRE(expected == target.on_stream_deleted_input);
   }
}
