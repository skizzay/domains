#include "domains/messaging/buffer.hpp"
#include <catch/catch.hpp>

using namespace domains;

TEST_CASE("reading and writing in memory", "[buffer]") {
   alignas(64) char memory[64] = {'A'};
   read_buffer consumer{&memory[0], sizeof(memory)};
   write_buffer producer{&memory[0], sizeof(memory)};

   REQUIRE(consumer.size() == producer.size());
   REQUIRE(consumer.available_to_read() == producer.available_to_write());
   REQUIRE(consumer.size() == sizeof(memory));

   SECTION("reading uint8_t reduces remaining by 1") {
      uint8_t x;
      REQUIRE(consumer.read_into(x) == std::error_code{});
      REQUIRE(consumer.available_to_read() == (sizeof(memory) - sizeof(uint8_t)));
   }

   SECTION("reading uint16_t reduces available_to_read by 2") {
      uint16_t x;
      REQUIRE(consumer.read_into(x) == std::error_code{});
      REQUIRE(consumer.available_to_read() == (sizeof(memory) - sizeof(uint16_t)));
   }

   SECTION("reading uint32_t reduces available_to_read by 4") {
      uint32_t x;
      REQUIRE(consumer.read_into(x) == std::error_code{});
      REQUIRE(consumer.available_to_read() == (sizeof(memory) - sizeof(uint32_t)));
   }

   SECTION("reading uint64_t reduces available_to_read by 8") {
      uint64_t x;
      REQUIRE(consumer.read_into(x) == std::error_code{});
      REQUIRE(consumer.available_to_read() == (sizeof(memory) - sizeof(uint64_t)));
   }

   SECTION("writing uint8_t reduces remaining by 1") {
      uint8_t x = 1;
      REQUIRE(producer.put(x) == std::error_code{});
      REQUIRE(producer.available_to_write() == (sizeof(memory) - sizeof(uint8_t)));
   }

   SECTION("writing uint16_t reduces available_to_write by 2") {
      uint16_t x = 1;
      REQUIRE(producer.put(x) == std::error_code{});
      REQUIRE(producer.available_to_write() == (sizeof(memory) - sizeof(uint16_t)));
   }

   SECTION("writing uint32_t reduces available_to_write by 4") {
      uint32_t x = 1;
      REQUIRE(producer.put(x) == std::error_code{});
      REQUIRE(producer.available_to_write() == (sizeof(memory) - sizeof(uint32_t)));
   }

   SECTION("writing uint64_t reduces available_to_write by 8") {
      uint64_t x = 1;
      REQUIRE(producer.put(x) == std::error_code{});
      REQUIRE(producer.available_to_write() == (sizeof(memory) - sizeof(uint64_t)));
   }
}
