#include <skizzay/domains/event_source/sequence.h>
#include <skizzay/domains/event_source/concepts.h>
#include <string>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace skizzay::domains::event_source;

using unsigned_sequenced = sequence<struct unsigned_sequenced_tag, std::make_unsigned_t<std::size_t>>;
using signed_sequenced = sequence<struct signed_sequenced_tag, std::make_signed_t<std::size_t>>;

TEST_CASE("Sequenced", "[event_source, sequence]") {
    SECTION("default constructed sequences are zero") {
        unsigned_sequenced const us;
        signed_sequenced const ss;

        REQUIRE(0 == us.value());
        REQUIRE(0 == ss.value());
    }

    SECTION("next produces the current value plus 1") {
        SECTION("for unsigned sequences") {
            unsigned_sequenced const current{3};
            REQUIRE(4 == current.next().value());
        }

        SECTION("for signed sequences") {
            signed_sequenced const current{3};
            REQUIRE(4 == current.next().value());
        }
    }

    SECTION("previous produces the current value minus 1") {
        SECTION("for unsigned sequences") {
            unsigned_sequenced const current{3};
            REQUIRE(2 == current.previous().value());
        }

        SECTION("for signed sequences") {
            signed_sequenced const current{3};
            REQUIRE(2 == current.previous().value());
        }
    }

    SECTION("sequence is sequenced") {
        REQUIRE(concepts::sequenced<signed_sequenced>);
        REQUIRE(concepts::sequenced<unsigned_sequenced>);
    }
}