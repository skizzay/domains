#include "skizzay/domains/event_sourcing/event_header.h"
#include <skizzay/kerchow/kerchow.h>
#include <skizzay/utilz/traits.h>

#include <catch.hpp>

using namespace skizzay::domains::event_sourcing;

TEST_CASE( "event_header", "[unit][event_header]" ) {
    using stream_id_type = basic_identifier<skizzay::utilz::tag<struct stream_id_type_tag>, int>;
    using stream_version_type = basic_stream_version<unsigned int>;
    using timestamp_type = std::chrono::high_resolution_clock::time_point;
    using target_type = event_header<stream_id_type, stream_version_type, timestamp_type>;

    SECTION( "passing in only the stream id sets other settings to 'not set'" ) {
        target_type const target{{}};

        REQUIRE( stream_id_type{} == target.stream_id() );
        REQUIRE( unsequenced<stream_version_type::tag_type, stream_version_type::value_type> == target.stream_version() );
        REQUIRE( timestamp_not_set<timestamp_type> == target.timestamp() );
    }

    SECTION( "passing in only the stream id sets other settings to 'not set'" ) {
        stream_id_type const stream_id{skizzay::kerchow::picker.pick(1)};
        stream_version_type const stream_version{skizzay::kerchow::picker.pick(1U)};
        timestamp_type const timestamp = std::chrono::high_resolution_clock::now();
        target_type const target{stream_id, stream_version, timestamp};

        REQUIRE( stream_id == target.stream_id() );
        REQUIRE( stream_version == target.stream_version() );
        REQUIRE( timestamp == target.timestamp() );
    }
}