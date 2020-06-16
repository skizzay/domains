#include "skizzay/domains/event_sourcing/identifier.h"
#include <skizzay/kerchow/kerchow.h>
#include "skizzay/utilz/traits.h"

#include <catch.hpp>

using namespace skizzay::domains::event_sourcing;
using namespace skizzay::utilz;

using test_identifier = basic_identifier<tag<struct test_tag>, std::size_t>;

TEST_CASE( "identifier constructor", "[unit][identifier]" ) {

    SECTION( "default construction is unidentified" ) {
        test_identifier const target;

        REQUIRE( unidentified<test_identifier::tag_type, test_identifier::value_type> == target );
        REQUIRE( test_identifier::value_type{} == target.value() );
    }

    SECTION( "construction using a value returns that value" ) {
        std::size_t const value = skizzay::kerchow::picker.pick(1U);
        test_identifier const target{value};

        REQUIRE( value == target.value() );
    }
}

TEST_CASE( "identifier concepts", "[unit][identifier]" ) {
    SECTION( "valid identifier is recognized properly when a valid identifier is tested" ) {
        REQUIRE( skizzay::utilz::is_template_v<test_identifier, basic_identifier> );
    }

    SECTION( "valid identifier is not recognized properly when an invalid identifier is tested" ) {
        REQUIRE( !skizzay::utilz::is_template_v<std::size_t, basic_identifier> );
    }
}