#include <skizzay/domains/aggregate/aggregate_root.h>
#include <skizzay/domains/aggregate/entity.h>
#include <skizzay/domains/sequence.h>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <memory_resource>
#include <iostream>

using namespace skizzay::domains::aggregate;

namespace {

using event_stream_id_type = std::string;
using event_stream_sequence_type = skizzay::domains::sequence<struct test, int>;
using event_stream_timestamp_type = std::chrono::high_resolution_clock::time_point;

struct test_event_a : skizzay::domains::event_source::tagged_event<test_event_a, event_stream_id_type, event_stream_sequence_type, event_stream_timestamp_type> {
   using skizzay::domains::event_source::tagged_event<test_event_a, event_stream_id_type, event_stream_sequence_type, event_stream_timestamp_type>::tagged_event;
};

struct test_event_b : skizzay::domains::event_source::tagged_event<test_event_b, event_stream_id_type, event_stream_sequence_type, event_stream_timestamp_type> {
   using skizzay::domains::event_source::tagged_event<test_event_b, event_stream_id_type, event_stream_sequence_type, event_stream_timestamp_type>::tagged_event;
};

struct test_event_c : skizzay::domains::event_source::tagged_event<test_event_c, event_stream_id_type, event_stream_sequence_type, event_stream_timestamp_type> {
   using skizzay::domains::event_source::tagged_event<test_event_c, event_stream_id_type, event_stream_sequence_type, event_stream_timestamp_type>::tagged_event;
};

using event_type = std::variant<test_event_a, test_event_b, test_event_c>;

struct test_aggregate_root : aggregate_root_base<test_aggregate_root, event_type> {
   DECLARE_DOMAINS_AGGREGATE_ROOT(test_aggregate_root, event_type);

   std::vector<event_type> applied_events;

   test_aggregate_root(event_stream_id_type entity_id) :
      aggregate_root_base<test_aggregate_root, event_type>{std::move(entity_id)}
   {
   }

   void make_a() {
      this->apply_event<test_event_a>();
   }

   void make_c() {
      this->apply_event<test_event_c>();
   }

private:
   void on(test_event_a const &a) {
      applied_events.push_back(a);
   }

   void on(test_event_b const &b) {
      applied_events.push_back(b);
   }

   // test_event_c is handled using the base class's "on" member function
};

using aggregate_version_type = test_aggregate_root::entity_version_type;

}

TEST_CASE("Aggregate Root", "[aggregate_root][aggregate]") {
   event_stream_id_type const entity_id{"test_aggregate_root_id"};
   test_aggregate_root target{entity_id};

   SECTION("starts off with version 0") {
      auto const actual = entity_version(target);
      REQUIRE(aggregate_version_type{} == actual);
   }

   SECTION("after adding an event") {
      target.make_a();

      SECTION("version is incremented") {
         REQUIRE(aggregate_version_type{1} == entity_version(target));
      }

      SECTION("the event was applied") {
         REQUIRE(1 == std::size(target.applied_events));
         REQUIRE(1 == std::size(uncommitted_events(target)));
         REQUIRE(0 == target.applied_events.back().index());
      }

      SECTION("creating an unhandled event") {
         target.make_c();

         SECTION("ensures that the event was not applied but is uncommitted") {
            REQUIRE(1 == std::size(target.applied_events));
            REQUIRE(2 == std::size(uncommitted_events(target)));
         }
      }

      SECTION("after clearing the uncommitted events") {
         clear_uncommitted_events(target);

         SECTION("uncommitted events are empty") {
            REQUIRE(std::empty(uncommitted_events(target)));
         }
      }
   }

   SECTION("after loading from history") {
      event_stream_timestamp_type const timestamp = std::chrono::high_resolution_clock::now();
      std::array<event_type, 3> const history{
         test_event_a{entity_id, event_stream_sequence_type{1}, timestamp},
         test_event_b{entity_id, event_stream_sequence_type{2}, timestamp},
         test_event_c{entity_id, event_stream_sequence_type{3}, timestamp}
      };
      load_aggregate_root_from_history(target, history);
      
      SECTION("there are no uncommitted events") {
         REQUIRE( std::ranges::empty(uncommitted_events(target)) );
      }
   }
}