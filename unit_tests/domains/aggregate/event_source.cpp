#include <domains/aggregate/event_source.hpp>
#include <kerchow/kerchow.h>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace domains;
using kerchow::picker;

TEST_CASE("Null event source", "[event_source, null_event_source]") {
   event_source<null_event_source> target;

   SECTION("does not return an error when saving an event") {
      REQUIRE(!target.save(char{}));
   }

   SECTION("num_events always returns 0") {
      REQUIRE(target.num_events(picker.pick<uint64_t>()) == std::size_t{0U});
   }
}
