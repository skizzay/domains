#include "skizzay/domains/event_sourcing/sequence.h"
#include <skizzay/kerchow/kerchow.h>
#include "skizzay/utilz/traits.h"

#include <catch.hpp>

using namespace skizzay::domains::event_sourcing;
using namespace skizzay::utilz;

TEST_CASE( "sequence constructor", "[unit][sequence]" ) {
    using test_sequence = basic_sequence<tag<struct test_tag>, std::size_t>;

    SECTION( "default construction is unsequenced" ) {
        test_sequence const target;

        REQUIRE( unsequenced<test_sequence::tag_type, test_sequence::value_type> == target );
        REQUIRE( test_sequence::value_type{} == target.value() );
    }

    SECTION( "construction using a value returns that value" ) {
        std::size_t const value = skizzay::kerchow::picker.pick(1U);
        test_sequence const target{value};

        REQUIRE( value == target.value() );
    }
}


TEST_CASE( "sequence concepts", "[unit][sequence]" ) {
    SECTION( "stream version" ) {
        REQUIRE( skizzay::utilz::is_template_v<basic_stream_version<unsigned int>, basic_sequence> );
    }
    SECTION( "commit sequence" ) {
        REQUIRE( skizzay::utilz::is_template_v<basic_commit_sequence<unsigned int>, basic_sequence> );
    }
}