#include "domains/messaging/buffer.hpp"
#if __has_include(<catch/catch.hpp>)
#  include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#  include <catch.hpp>
#endif

using namespace domains;
using ENCODING = domains::encode_native_endian;

TEST_CASE("reading and writing in memory", "[buffer]") {
   alignas(64) char memory[64] = {'A'};
   read_buffer<ENCODING> consumer{&memory[0], sizeof(memory)};
   write_buffer<ENCODING> producer{&memory[0], sizeof(memory)};

   REQUIRE(consumer.size() == producer.size());
   REQUIRE(consumer.remaining() == producer.remaining());
   REQUIRE(consumer.size() == sizeof(memory));

   SECTION("reading next uint8_t reduces remaining by 1") {
      uint8_t x;
      REQUIRE(consumer.read_next(x) == std::error_code{});
      REQUIRE(consumer.remaining() == (sizeof(memory) - sizeof(uint8_t)));
   }

   SECTION("reading next (exceptional) uint8_t reduces remaining by 1") {
      uint8_t x = consumer.read_next();
      REQUIRE(consumer.remaining() == (sizeof(memory) - sizeof(uint8_t)));
   }

   SECTION("reading indexed uint8_t does not reduce remaining") {
      uint8_t x;
      REQUIRE(consumer.read_at(x, 0) == std::error_code{});
      REQUIRE(consumer.remaining() == consumer.size());
   }

   SECTION("reading uint16_t reduces remaining by 2") {
      uint16_t x;
      REQUIRE(consumer.read_next(x) == std::error_code{});
      REQUIRE(consumer.remaining() == (sizeof(memory) - sizeof(uint16_t)));
   }

   SECTION("reading indexed uint16_t does not reduce remaining") {
      uint16_t x;
      REQUIRE(consumer.read_at(x, 0) == std::error_code{});
      REQUIRE(consumer.remaining() == consumer.size());
   }

   SECTION("reading uint32_t reduces remaining by 4") {
      uint32_t x;
      REQUIRE(consumer.read_next(x) == std::error_code{});
      REQUIRE(consumer.remaining() == (sizeof(memory) - sizeof(uint32_t)));
   }

   SECTION("reading indexed uint32_t does not reduce remaining") {
      uint32_t x;
      REQUIRE(consumer.read_at(x, 0) == std::error_code{});
      REQUIRE(consumer.remaining() == consumer.size());
   }

   SECTION("reading uint64_t reduces remaining by 8") {
      uint64_t x;
      REQUIRE(consumer.read_next(x) == std::error_code{});
      REQUIRE(consumer.remaining() == (sizeof(memory) - sizeof(uint64_t)));
   }

   SECTION("reading indexed uint64_t does not reduce remaining") {
      uint64_t x;
      REQUIRE(consumer.read_at(x, 0) == std::error_code{});
      REQUIRE(consumer.remaining() == consumer.size());
   }

   SECTION("reading a write_buffer sets the read_buffer to the buffer position") {
      // Putting 1 byte
      producer.put_next(uint8_t('A'));
      REQUIRE(producer.read().size() == 1);
      REQUIRE(producer.read().remaining() == 1);
   }

   SECTION("reading a write_buffer sets the value to what was written") {
      const uint16_t expected = 0x0A0B;
      producer.put_next(expected);
      uint16_t actual = producer.read().read_next();
      REQUIRE(actual == expected);
   }
}
