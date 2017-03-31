#include "domains/messaging/decoder.hpp"
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif
#include <algorithm>
#include <experimental/string_view>
#include <iostream>

using namespace domains;

namespace {
class data {
   std::vector<byte> bytes;

public:
   explicit data(read_buffer<native_endian_encoding> buffer) : bytes(buffer.remaining()) {
      buffer.read_into(bytes.data(), bytes.size());
   }

   read_buffer<native_endian_encoding> read() const noexcept {
      return {bytes.data(), bytes.size()};
   }
};

struct A {
   virtual std::string name() const = 0;
};

struct B final : A {
   std::string value;
   virtual std::string name() const override {
      return "B::" + value;
   }
};

struct C : A {
   std::string value;
   virtual std::string name() const override {
      return "C::" + value;
   }
};

struct D final {};

enum class data_type : uint8_t { A, B, C, D };

struct router {
   template <class F>
   std::error_code operator()(data const &d, F &&decode_and_dispatch) noexcept {
      auto buffer = d.read();
      data_type type = buffer.read();
      switch (type) {
      case data_type::B:
         return decode_and_dispatch(buffer, decode_type<B>{});

      case data_type::C:
         return decode_and_dispatch(buffer, decode_type<C>{});

      case data_type::D:
         return decode_and_dispatch(buffer, decode_type<D>{});

      default:
         break;
      }
      return make_error_code(std::errc::not_supported);
   }
};
}

TEST_CASE("Decoder", "[decoder]") {
   using EncodedType = read_buffer<native_endian_encoding>;
   byte memory[64] = {{}};
   write_buffer<native_endian_encoding> writer{&memory[0], sizeof(memory)};
   bool a_found = false;
   bool b_found = false;
   bool c_found = false;
   bool d_found = false;
   bool other_found = false;
   std::string name;
   auto concrete_dispatcher = make_single_dispatcher(
       [&b_found, &name](B const &b) -> std::error_code {
          b_found = true;
          name = b.name();
          return {};
       },
       [&c_found, &name](C const &c) -> std::error_code {
          c_found = true;
          name = c.name();
          return {};
       },
       [&d_found](D const &) -> std::error_code {
          d_found = true;
          return {};
       });
   auto base_dispatcher = make_single_dispatcher(
       [&a_found, &name](A const &a) -> std::error_code {
          a_found = true;
          name = a.name();
          return {};
       },
       [&d_found](D const &) -> std::error_code {
          d_found = true;
          return {};
       });
   auto catch_all_dispatcher = [](auto const &) { return std::error_code{}; };

   auto decode_dispatcher = make_single_dispatcher(
       [](EncodedType, B &decoded) {
          decoded.value = "DDD";
          return std::error_code{};
       },
       [](EncodedType, C &decoded) {
          decoded.value = "DDD";
          return std::error_code{};
       },
       [](EncodedType, D &) { return std::error_code{}; });
   auto target = decoder<router, decltype(decode_dispatcher), B, C, D>{
       router{}, std::move(decode_dispatcher)};

   SECTION("can decode concrete types provided") {
      writer << data_type::B;
      data input{writer.read()};
      auto error = target(concrete_dispatcher, input);
      REQUIRE(!error);
      REQUIRE(name == "B::DDD");
      REQUIRE(!a_found);
      REQUIRE(b_found);
      REQUIRE(!c_found);
      REQUIRE(!d_found);
   }

   SECTION("can decode base types provided") {
      writer << data_type::C;
      data input{writer.read()};
      auto error = target(base_dispatcher, input);
      REQUIRE(!error);
      REQUIRE(name == "C::DDD");
      REQUIRE(a_found);
      REQUIRE(!b_found);
      REQUIRE(!c_found);
      REQUIRE(!d_found);
   }

   SECTION("can decode simple types provided") {
      writer << data_type::D;
      data input{writer.read()};
      auto error = target(base_dispatcher, input);
      REQUIRE(!error);
      REQUIRE(name.empty());
      REQUIRE(!a_found);
      REQUIRE(!b_found);
      REQUIRE(!c_found);
      REQUIRE(d_found);

      d_found = false;
      error = target(concrete_dispatcher, data{writer.read()});
      REQUIRE(!error);
      REQUIRE(name.empty());
      REQUIRE(!a_found);
      REQUIRE(!b_found);
      REQUIRE(!c_found);
      REQUIRE(d_found);
   }

   SECTION("can decode using fallback") {
      writer << data_type::D;
      auto error = target(catch_all_dispatcher, data{writer.read()});
      REQUIRE(!error);
      REQUIRE(name.empty());
      REQUIRE(!a_found);
      REQUIRE(!b_found);
      REQUIRE(!c_found);
      REQUIRE(!d_found);
   }
}
