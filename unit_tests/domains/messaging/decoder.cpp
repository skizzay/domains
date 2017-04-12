#include "domains/messaging/decoder.hpp"

#include "unit_tests/domains/messaging/base_fixture.hpp"
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <algorithm>
#include <experimental/string_view>
#include <iostream>

using namespace domains;

TEST_CASE("Decoder", "[decoder]") {
   byte memory[64] = {{}};
   write_buffer<native_endian_encoding> writer{&memory[0], sizeof(memory)};
   factory f;
   auto target = f.make_decoder();

   SECTION("can decode concrete types provided") {
      writer << data_type::B;
      data input{writer.read()};
      auto error = target(f.make_single_domain_dispatcher(), input);
      REQUIRE(!error);
      REQUIRE(f.a_count == 0);
      REQUIRE(f.b_count == 1);
      REQUIRE(f.c_count == 0);
      REQUIRE(f.d_count == 0);
      REQUIRE(f.e_count == 0);
   }

   SECTION("can decode base types provided") {
      writer << data_type::G;
      data input{writer.read()};
      auto error = target(f.make_single_domain_dispatcher(), input);
      REQUIRE(!error);
      REQUIRE(f.a_count == 1);
      REQUIRE(f.b_count == 0);
      REQUIRE(f.c_count == 0);
      REQUIRE(f.d_count == 0);
      REQUIRE(f.e_count == 0);
   }

   SECTION("can decode simple types provided") {
      writer << data_type::D;
      data input{writer.read()};
      auto error = target(f.make_single_domain_dispatcher(), input);
      REQUIRE(!error);
      REQUIRE(f.a_count == 0);
      REQUIRE(f.b_count == 0);
      REQUIRE(f.c_count == 0);
      REQUIRE(f.d_count == 1);
      REQUIRE(f.e_count == 0);

      error = target(f.make_single_primitive_dispatcher(), data{writer.read()});
      REQUIRE(!error);
      REQUIRE(f.a_count == 0);
      REQUIRE(f.b_count == 0);
      REQUIRE(f.c_count == 0);
      REQUIRE(f.d_count == 2);
      REQUIRE(f.e_count == 0);
   }

   SECTION("can decode using fallback") {
      writer << data_type::D;
      auto error = target(f.make_catchall_dispatcher(), data{writer.read()});
      REQUIRE(!error);
      REQUIRE(f.a_count == 0);
      REQUIRE(f.b_count == 0);
      REQUIRE(f.c_count == 0);
      REQUIRE(f.d_count == 0);
      REQUIRE(f.e_count == 0);
   }
}
