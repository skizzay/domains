#include "skizzay/domains/event_sourcing/event.h"
#include <skizzay/kerchow/kerchow.h>
#include <skizzay/utilz/traits.h>
#include <catch.hpp>

using namespace skizzay::domains::event_sourcing;

namespace {

struct event_body {
    int int_field = {};
};


using stream_id_type = basic_identifier<skizzay::utilz::tag<struct stream_id_type_tag>, int>;
using stream_version_type = basic_stream_version<unsigned int>;
using timestamp_type = std::chrono::high_resolution_clock::time_point;
using header_type = basic_event_header<stream_id_type, stream_version_type, timestamp_type>;
using target_type = basic_event<header_type, skizzay::utilz::tag<struct event_tag>, event_body>;

}

TEST_CASE( "event body details", "[unit][event]" ) {
    stream_id_type const stream_id{skizzay::kerchow::picker.pick(1)};
    event_body const body{skizzay::kerchow::picker.pick<int>()};
    target_type target{{stream_id}, body};

    SECTION( "Constructor taking in header and body makes fields top-level accessible" ) {
        REQUIRE( stream_id == target.stream_id() );
        REQUIRE( body.int_field == target.int_field );
    }
}

TEST_CASE( "event concepts", "[unit][event][concepts]" ) {
    SECTION( "target type is an event" ) {
        REQUIRE( concepts::event<target_type> );
    }
    SECTION( "vector of target types is an event range" ) {
        REQUIRE( concepts::event_range<std::vector<target_type>> );
    }
}