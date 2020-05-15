#include "skizzay/domains/event_sourcing/commit.h"
#include "skizzay/domains/event_sourcing/commit_attempt.h"
#include "skizzay/domains/event_sourcing/commit_header.h"
#include "skizzay/domains/event_sourcing/commit_result.h"
#include "skizzay/domains/event_sourcing/commit_store.h"
#include "skizzay/domains/event_sourcing/hook.h"
#include "skizzay/domains/event_sourcing/traits.h"
#include <algorithm>
#include <catch.hpp>
#include <chrono>
#include <cstdint>
#include <skizzay/kerchow/kerchow.h>
#include <skizzay/utilz/dto.h>
#include <string>
#include <vector>

using namespace skizzay;
using namespace skizzay::domains::event_sourcing;

namespace {

using event_range_type = std::vector<std::string>;
using stream_id_type = std::uint64_t;
using stream_version_type = basic_stream_version<std::uint64_t>;
using commit_id_type = std::uint64_t;
using commit_sequence_type = basic_commit_sequence<std::uint32_t>;
using timestamp_type = std::chrono::system_clock::time_point;
using commit_header = basic_commit_header<stream_id_type, stream_version_type, commit_id_type,
                                          commit_sequence_type, timestamp_type>;
using commit_attempt = basic_commit_attempt<commit_header, event_range_type>;
using commit = basic_commit<commit_header, event_range_type>;

struct fake_commit_store {
   using timestamp_type = ::timestamp_type;
   using commit_id_type = ::commit_id_type;
   using stream_id_type = ::stream_id_type;
   using stream_version_type = ::stream_version_type;
   using commit_sequence_type = ::commit_sequence_type;

   mutable std::vector<stream_id_type> ids;
   std::vector<commit> commits;
   bool should_allow_put = true;

   std::vector<commit> get_commits(stream_id_type const &sid,
                                   stream_version_type const min_sequence,
                                   stream_version_type const max_sequence) const {
      ids.push_back(sid);
      return {begin(commits) + min_sequence.value(),
              begin(commits) + std::min(std::size(commits), max_sequence.value())};
   }

   template <class EventRange>
   commit put(basic_commit_attempt<commit_header, EventRange> const &attempt) {
      if (should_allow_put) {
         commit const c{static_cast<commit_header const &>(attempt),
                        event_range_type{std::begin(attempt), std::end(attempt)},
                        commit_result::success};
         commits.push_back(c);
         return c;
      } else {
         return commit{attempt, commit_result::internal_error, get_empty_range()};
      }
   }

   event_range_type get_empty_range() const noexcept {
      return {};
   }
};

struct commit_observer {
   bool result = true;
   mutable std::size_t num_attempts = 0;
   mutable std::size_t num_commits = 0;

   template <class Attempt>
   constexpr bool on_commit_attempt(Attempt const &) const noexcept {
      ++num_attempts;
      return result;
   }

   template <class Commit>
   constexpr void on_commit(Commit const &) noexcept {
      ++num_commits;
   }
};

event_range_type create_events() {
   auto c{kerchow::picker.create_fuzzy_container<std::string>(
       {std::string{"hello"}, std::string{"world"}, std::string{"utilz"}, std::string{"fsm"},
        std::string{"kerchow"}, std::string{"domains"}, std::string{"cqrs"},
        std::string{"event_sourcing"}})};
   c.set_iteration_count(kerchow::picker.pick(1, c.size()));
   return {std::begin(c), std::end(c)};
}

} // namespace

TEST_CASE("commit_store::traits", "[unit][commit_store]") {
   SECTION("get_commits") {
      REQUIRE(utilz::is_detected_v<details_::commit_store_get_commits_t, fake_commit_store>);
      REQUIRE(utilz::is_range_v<
              utilz::detected_t<details_::commit_store_get_commits_t, fake_commit_store>>);
   }

   SECTION("typename timestamp_type") {
      REQUIRE(utilz::is_detected_v<details_::typename_timestamp_type, fake_commit_store>);
      REQUIRE(utilz::is_time_point_v<
              utilz::detected_t<details_::typename_timestamp_type, fake_commit_store>>);
   }
}

TEST_CASE("commit_store::put", "[unit][commit_store]") {
   commit_observer observe_commits;

   SECTION("Setup") {
      std::uint64_t const stream_id = kerchow::picker();
      std::uint32_t const stream_sequence = kerchow::picker();
      std::uint64_t const commit_id = kerchow::picker();
      std::uint32_t const commit_sequence = kerchow::picker();
      std::chrono::system_clock::time_point const timestamp = std::chrono::system_clock::now();
      basic_commit_header header{stream_id, stream_version_type{stream_sequence}, commit_id, commit_sequence_type{commit_sequence}, timestamp};
      event_range_type expected_events{create_events()};
      basic_commit_attempt attempt{header, expected_events};
      fake_commit_store fake;
      commit_store target{std::ref(fake), hook{std::ref(observe_commits)}};

      REQUIRE(0 == observe_commits.num_attempts);
      REQUIRE(0 == observe_commits.num_commits);
      REQUIRE(0 == target.get_commits(stream_id).size());

      auto verify_put = [&](std::size_t const expected_num_commits,
                            commit_result const expected_result,
                            commit const &actual_commit) {
         std::size_t const expected_num_attempts = 1;

         REQUIRE(expected_num_attempts == observe_commits.num_attempts);
         REQUIRE(expected_num_commits == observe_commits.num_commits);
         REQUIRE(expected_num_commits == target.get_commits(stream_id).size());
         REQUIRE(expected_result == actual_commit.result_value());
         REQUIRE(std::equal(std::begin(expected_events), std::end(expected_events),
                            std::begin(actual_commit), std::end(actual_commit)));
      };

      SECTION("Successful commit") {
         std::size_t const expected_num_commits = 1;

         auto const actual_result{target.put(attempt)};

         verify_put(expected_num_commits, commit_result::success, actual_result);
      }

      SECTION("Unsuccessful commit") {
         std::size_t const expected_num_commits = 0;
         expected_events.clear();
         fake.should_allow_put = false;

         auto const actual_result{target.put(attempt)};

         verify_put(expected_num_commits, commit_result::internal_error, actual_result);
      }

      SECTION("Pipeline invalidated") {
         std::size_t const expected_num_commits = 0;
         expected_events.clear();
         observe_commits.result = false;

         auto const actual_result{target.put(attempt)};

         verify_put(expected_num_commits, commit_result::pipeline_invalidated, actual_result);
      }
   }
}
