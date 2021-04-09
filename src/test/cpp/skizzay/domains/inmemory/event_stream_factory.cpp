#include <skizzay/domains/inmemory/event_stream_factory.h>
#include <skizzay/domains/event_stream.h>
#include <skizzay/domains/null_mutex.h>
#include <skizzay/domains/sequence.h>
#include <chrono>
#include <vector>
#include <iostream>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains;
using namespace std::literals;
using mutex_type = null_mutex<true>;

namespace {

using stream_id_type = std::string;
using stream_sequence_type = sequence<struct test, std::size_t>;
using stream_timestamp_type = std::chrono::high_resolution_clock::time_point;
using test_event = tagged_event<struct test, stream_id_type, stream_sequence_type, stream_timestamp_type>;
using commit_timestamp_provider = decltype(&std::chrono::high_resolution_clock::now);

auto make_target = [](auto commit_id_provider) {
   using target_type = inmemory::event_stream_store<test_event, decltype(commit_id_provider), commit_timestamp_provider, mutex_type>;
   return target_type(
      std::move(commit_id_provider),
      &std::chrono::high_resolution_clock::now
   );
};
}

TEST_CASE("In-Memory Event Store", "[event_source, event_stream_factory, event_stream]") {
   int commit_number = 0;
   auto target = make_target([commit_number]() mutable {
      return "commit_id_" + std::to_string(++commit_number); 
   });

   SECTION("Creating an event stream") {
      SECTION("when not previously created works") {
         CHECK_NOTHROW(target.get_event_stream("event_stream_id"));

         SECTION("and can be recreated if nothing was persisted") {
            CHECK_NOTHROW(target.get_event_stream("event_stream_id"));
         }

         SECTION("and can create another stream with different id") {
            CHECK_NOTHROW(target.get_event_stream("different_id"));
         }
      }
   }

   SECTION("Utilizing the event stream") {
      auto stream_id = "event_stream_id"s;
      auto event_stream = target.get_event_stream(stream_id);

      SECTION("is initially empty") {
         auto r = events(event_stream);
         REQUIRE(std::begin(r) == std::end(r));
      }

      SECTION("given raw_events with unexpected starting sequence") {
         std::vector raw_events{test_event{stream_id, stream_sequence_type{22}}, test_event{stream_id, stream_sequence_type{23}}};

         SECTION("when we put items into the stream") {
            auto commit = put_event_stream(target, event_stream, raw_events);
            SECTION("then a concurrency collision was encountered") {
               REQUIRE(commit.error());
            }
         }
      }

      SECTION("given raw_events with gaps in the sequnce sequence") {
         std::vector raw_events{test_event{stream_id, stream_sequence_type{1}}, test_event{stream_id, stream_sequence_type{23}}};

         SECTION("when we put items into the stream") {
            auto commit = put_event_stream(target, event_stream, raw_events);
            SECTION("then a concurrency collision was encountered") {
               REQUIRE(commit.error());
            }
         }
      }

      SECTION("given raw_events with a stream id that doesn't match the stream's id") {
         std::vector raw_events{test_event{stream_id + " not the same", stream_sequence_type{1}}, test_event{stream_id, stream_sequence_type{2}}};

         SECTION("when we put items into the stream") {
            auto commit = put_event_stream(target, event_stream, raw_events);
            SECTION("then an invalid argument was encountered") {
               REQUIRE(commit.error());
            }
         }
      }

      SECTION("given valid raw_events") {
         std::vector raw_events{test_event{stream_id, stream_sequence_type{1}}, test_event{stream_id, stream_sequence_type{2}}};

         SECTION("when we put items into the stream") {
            auto commit = put_event_stream(target, event_stream, raw_events);

            SECTION("then the commit will have the begin and end sequence of the events put into the stream") {
               REQUIRE(stream_sequence_type{1} == commit.event_stream_starting_sequence());
               REQUIRE(stream_sequence_type{2} == commit.event_stream_ending_sequence());
            }

            SECTION("and the commit will have the same event_stream_id") {
               REQUIRE(event_stream_id(commit) == stream_id);
            }

            SECTION("when we retrieve the events") {
               std::vector<test_event> actual_events;
               std::ranges::copy(events(event_stream, stream_sequence_type{1}, stream_sequence_type{3}), std::back_inserter(actual_events));

               SECTION("then all of the events have been returned") {
                  REQUIRE(2 == std::ranges::size(actual_events));
               }

               SECTION("and the timestamp matches the commit timestamp") {
                  auto filtered_events = actual_events | std::ranges::views::filter(
                     [commit_timestamp=commit.commit_timestamp()](test_event const &e) noexcept {
                        return commit_timestamp == event_stream_timestamp(e);
                     }
                  );
                  REQUIRE(2 == std::distance(std::ranges::begin(filtered_events), std::ranges::end(filtered_events)));
               }
            }
         }
      }
   }
}