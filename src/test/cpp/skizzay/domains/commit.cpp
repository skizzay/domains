#include <skizzay/domains/commit.h>
#include <skizzay/domains/event.h>
#include <skizzay/domains/sequence.h>
#include <string>
#include <chrono>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains;

namespace {
using test_sequence = sequence<struct test, std::uint16_t>;
struct test_event : tagged_event<test_event, std::string, test_sequence, std::chrono::steady_clock::time_point> {
   using tagged_event<test_event, std::string, test_sequence, std::chrono::steady_clock::time_point>::tagged_event;
};
using test_commit = basic_commit<std::string, event_stream_id_t<test_event>, event_stream_sequence_t<test_event>, event_stream_timestamp_t<test_event>, std::exception_ptr>;
}

TEST_CASE("Commit concept", "[event_source, commit]") {
   SECTION("basic_commit is a commit") {
      REQUIRE(concepts::commit<test_commit>);
   }
}

TEST_CASE("Successful commit", "[event_source, commit]") {
   std::string const commit_id{"commit_id"};
   std::string const event_stream_id{"event_string_id"};
   auto const timestamp = std::chrono::steady_clock::now();
   std::vector const events = {test_event{event_stream_id, test_sequence{3}, timestamp}, test_event{event_stream_id, test_sequence{4}, timestamp + std::chrono::seconds{1}}};
   test_commit const target{commit_id, event_stream_id, timestamp, test_sequence{3}, test_sequence{4}};

   SECTION("commit_id should be pass-thru") {
      REQUIRE(commit_id == target.commit_id());
   }

   SECTION("commit_timestamp should be pass-thru") {
      REQUIRE(timestamp == target.commit_timestamp());
   }

   SECTION("should not be an error") {
      REQUIRE(!target.is_error());
      REQUIRE(!target.error());
   }
}

TEST_CASE("Error commit", "[event_source, commit]") {
   std::string const commit_id{"commit_id"};
   std::string const event_stream_id{"event_string_id"};
   auto const timestamp = std::chrono::steady_clock::now();
   test_commit const target{commit_id, event_stream_id, timestamp, std::make_exception_ptr(std::invalid_argument{"value"})};

   SECTION("commit_id should be pass-thru") {
      REQUIRE(commit_id == target.commit_id());
   }

   SECTION("commit_timestamp should be pass-thru") {
      REQUIRE(timestamp == target.commit_timestamp());
   }

   SECTION("should be an error") {
      REQUIRE(target.is_error());
      REQUIRE(target.error());
   }

   SECTION("accessing non-error throws") {
      try {
         target.event_stream_starting_sequence();
         FAIL("Expected invalid_argument exception");
      }
      catch (std::invalid_argument const &e) {
         return;
      }
      catch (...) {
         FAIL("Unexpected exception");
      }
   }
}