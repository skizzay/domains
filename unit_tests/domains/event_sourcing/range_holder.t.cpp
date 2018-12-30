#include "skizzay/domains/event_sourcing/range_holder.h"
#include <skizzay/kerchow/kerchow.h>
#include <catch.hpp>
#include <functional>
#include <vector>

#include <iostream>

using namespace skizzay;
using namespace skizzay::domains::event_sourcing;

namespace {

inline std::vector<int> get_range() {
   auto const c = kerchow::picker.create_fuzzy_container<int>({1, 2, 3, 4, 5});
   return {std::begin(c), std::end(c)};
}

}

TEST_CASE( "range_holder::copy", "[unit][range_holder]" ) {
   std::vector<int> const ints = get_range();
   std::size_t const size = std::size(ints);
   range_holder target{ints, size};

   SECTION( "size and count" ) {
      REQUIRE( std::size(ints) == std::size(target) );
      REQUIRE( std::size(ints) == std::distance(std::begin(target), std::end(target)) );
   }

   SECTION( "empty" ) {
      REQUIRE_FALSE( std::empty(target) );
   }
}

TEST_CASE( "range_holder::reference_wrapper", "[unit][range_holder]" ) {
   std::vector<int> const ints = get_range();
   range_holder target{std::ref(ints), std::size(ints)};

   SECTION( "size and count" ) {
      REQUIRE( std::size(ints) == std::size(target) );
      REQUIRE( std::size(ints) == std::distance(std::begin(target), std::end(target)) );
   }

   SECTION( "empty" ) {
      REQUIRE_FALSE( std::empty(target) );
   }
}

TEST_CASE( "range_holder::move", "[unit][range_holder]" ) {
   std::vector<int> ints = get_range();
   std::size_t const size = std::size(ints);
   range_holder<std::vector<int>> target{std::move(ints), size};

   {
      std::size_t index = 0;
      for (int const i : target) {
         std::cout << "target[" << index << "]=" << i << std::endl;
         ++index;
      }
   }

   SECTION( "size and count" ) {
      REQUIRE( size == std::size(target) );
      REQUIRE( size == std::distance(std::begin(target), std::end(target)) );
   }

   SECTION( "empty" ) {
      REQUIRE_FALSE( std::empty(target) );
   }

   std::cout << "Done" << std::endl;
}
