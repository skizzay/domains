#include "domains/messaging/encoding.hpp"
#include <kerchow/kerchow.h>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace domains;
using kerchow::picker;

using ENCODING = domains::native_endian_encoding;

namespace {

bool const is_big_endian = [] {
   short const x = 0xABCD;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
   return 0xAB == reinterpret_cast<char const *>(&x)[0];
#pragma GCC diagnostic pop
}();

TEST_CASE("Encoding and decoding", "[encoding]") {
   alignas(std::max_align_t) uint8_t same_memory[sizeof(std::uintmax_t)] = {0};
   alignas(std::max_align_t) uint8_t diff_memory[sizeof(std::uintmax_t)] = {0};
   alignas(std::max_align_t) uint8_t native_memory[sizeof(std::uintmax_t)] = {0};
   uint8_t u8 = picker.pick<uint8_t>();
   uint16_t u16 = picker.pick<uint16_t>();
   uint32_t u32 = picker.pick<uint32_t>();
   uint64_t u64 = picker.pick<uint64_t>();

   auto encode_and_test = [&](auto const value) {
      native_endian_encoding::encode(value, native_memory);
      if (is_big_endian) {
         big_endian_encoding::encode(value, same_memory);
         little_endian_encoding::encode(value, diff_memory);
      } else {
         big_endian_encoding::encode(value, diff_memory);
         little_endian_encoding::encode(value, same_memory);
      }
      REQUIRE(std::equal(&native_memory[0], &native_memory[sizeof(value)], &same_memory[0],
                         &same_memory[sizeof(value)]));
      REQUIRE(std::equal(&native_memory[0], &native_memory[sizeof(value)],
                         std::make_reverse_iterator(&diff_memory[sizeof(value)]),
                         std::make_reverse_iterator(&diff_memory[0])));
   };

   auto native_decode_test = [&](auto const value) {
      std::remove_const_t<decltype(value)> new_value = ~value;
      native_endian_encoding::encode(value, native_memory);
      if (is_big_endian) {
         big_endian_encoding::decode(new_value, &native_memory[0]);
      } else {
         little_endian_encoding::decode(new_value, &native_memory[0]);
      }
      REQUIRE(value == new_value);
   };

   auto encode_decode_test = [&](auto const value, auto encoding) {
      std::remove_const_t<decltype(value)> new_value = ~value;
      decltype(encoding)::encode(value, &native_memory[0]);
      decltype(encoding)::decode(new_value, &native_memory[0]);
      REQUIRE(value == new_value);
   };

   SECTION("encodeting is encoded properly") {
      encode_and_test(u8);
      encode_and_test(u16);
      encode_and_test(u32);
      encode_and_test(u64);
   }

   SECTION("decodeing from encode value returns same value") {
      native_decode_test(u8);
      native_decode_test(u16);
      native_decode_test(u32);
      native_decode_test(u64);
   }

   SECTION("decodeing from encode value returns same value") {
      encode_decode_test(u8, big_endian_encoding{});
      encode_decode_test(u16, big_endian_encoding{});
      encode_decode_test(u32, big_endian_encoding{});
      encode_decode_test(u64, big_endian_encoding{});
      encode_decode_test(u8, little_endian_encoding{});
      encode_decode_test(u16, little_endian_encoding{});
      encode_decode_test(u32, little_endian_encoding{});
      encode_decode_test(u64, little_endian_encoding{});
      encode_decode_test(u8, native_endian_encoding{});
      encode_decode_test(u16, native_endian_encoding{});
      encode_decode_test(u32, native_endian_encoding{});
      encode_decode_test(u64, native_endian_encoding{});
   }
}
}
