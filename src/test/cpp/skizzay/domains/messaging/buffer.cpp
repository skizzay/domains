#include "domains/messaging/buffer.hpp"
#include <kerchow/kerchow.h>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace domains;
using kerchow::picker;

TEST_CASE("Reading from a buffer", "[read_buffer]") {
   alignas(64) char memory[64];
   read_buffer<native_endian_encoding> target{&memory[0], sizeof(memory)};
   char s[8];
   std::uint8_t u8 = 0;
   std::uint16_t u16 = 0;
   std::uint32_t u32 = 0;
   std::uint64_t u64 = 0;
   std::int8_t s8 = 0;
   std::int16_t s16 = 0;
   std::int32_t s32 = 0;
   std::int64_t s64 = 0;

   std::memset(memory, 0x81, sizeof(memory));
   std::memset(s, 0, sizeof(s));
   REQUIRE(sizeof(memory) == target.size());

   SECTION("sets the values read from the buffer") {
      target >> u8 >> u16 >> u32 >> u64;
      REQUIRE(u8 == 0x81);
      REQUIRE(u16 == 0x8181);
      REQUIRE(u32 == 0x81818181);
      REQUIRE(u64 == 0x8181818181818181);
      target >> s8 >> s16 >> s32 >> s64;
      target.read_into(s, sizeof(s));
   }

   SECTION("causes the current index to move by size of the value") {
      std::size_t expected = 0U;
      REQUIRE(expected == target.current());
      target >> u8;
      expected += sizeof(u8);
      REQUIRE(expected == target.current());
      target >> u16;
      expected += sizeof(u16);
      REQUIRE(expected == target.current());
      target >> u32;
      expected += sizeof(u32);
      REQUIRE(expected == target.current());
      target >> u64;
      expected += sizeof(u64);
      REQUIRE(expected == target.current());
      target >> s8;
      expected += sizeof(s8);
      REQUIRE(expected == target.current());
      target >> s16;
      expected += sizeof(s16);
      REQUIRE(expected == target.current());
      target >> s32;
      expected += sizeof(s32);
      REQUIRE(expected == target.current());
      target >> s64;
      expected += sizeof(s64);
      REQUIRE(expected == target.current());
      target.read_into(s, sizeof(s));
      expected += sizeof(s);
      REQUIRE(expected == target.current());
   }

   SECTION("causes the remaining to reduce by size of the value") {
      std::size_t expected = sizeof(memory);
      REQUIRE(expected == target.remaining());
      target >> u8;
      expected -= sizeof(u8);
      REQUIRE(expected == target.remaining());
      target >> u16;
      expected -= sizeof(u16);
      REQUIRE(expected == target.remaining());
      target >> u32;
      expected -= sizeof(u32);
      REQUIRE(expected == target.remaining());
      target >> u64;
      expected -= sizeof(u64);
      REQUIRE(expected == target.remaining());
      target >> s8;
      expected -= sizeof(s8);
      REQUIRE(expected == target.remaining());
      target >> s16;
      expected -= sizeof(s16);
      REQUIRE(expected == target.remaining());
      target >> s32;
      expected -= sizeof(s32);
      REQUIRE(expected == target.remaining());
      target >> s64;
      expected -= sizeof(s64);
      REQUIRE(expected == target.remaining());
      target.read_into(s, sizeof(s));
      expected -= sizeof(s);
      REQUIRE(expected == target.remaining());
   }

   SECTION("keeps index in place when directly referencing location") {
      target.skip(picker.pick(std::size_t{}, target.remaining() / 2U));
      std::size_t const expected = target.current();
      std::size_t const ignored = target.read_at(expected + 1);
      REQUIRE(expected == target.current());
      target.read_into_at(s, sizeof(s), 3);
      REQUIRE(expected == target.current());
   }

   SECTION("can have index updated when specifically set") {
      target.skip(picker.pick(std::size_t{1}, target.remaining() - 1U));
      REQUIRE(target.set_current(0));
      REQUIRE(0U == target.current());
      REQUIRE(!target.set_current(picker.pick(target.size())));
      REQUIRE(0U == target.current());
   }

   SECTION("will throw system error when checked and no more memory is available") {
      REQUIRE_NOTHROW(checked(target >> u8));
      target.skip(target.remaining());
      REQUIRE_THROWS_AS(checked(target >> u8), std::system_error);
   }
}


TEST_CASE("Writing to a buffer", "[write_buffer]") {
   alignas(64) char memory[64];
   write_buffer<native_endian_encoding> target{&memory[0], sizeof(memory)};
   char s[8];
   std::uint8_t u8 = 0x81;
   std::uint16_t u16 = 0x8181;
   std::uint32_t u32 = 0x81818181;
   std::uint64_t u64 = 0x8181818181818181;
   std::int8_t s8 = 0x41;
   std::int16_t s16 = 0x4141;
   std::int32_t s32 = 0x41414141;
   std::int64_t s64 = 0x4141414141414141;
   constexpr std::size_t const u_size = sizeof(u8) + sizeof(u16) + sizeof(u32) + sizeof(u64);
   constexpr std::size_t const s_size = sizeof(s8) + sizeof(s16) + sizeof(s32) + sizeof(s64);
   constexpr std::size_t const a_size = u_size + s_size + sizeof(s);

   std::memset(memory, 0, sizeof(memory));
   std::memset(s, 0x46, sizeof(s));
   REQUIRE(sizeof(memory) == target.size());

   auto test_range = [](char const *b, char const *e, char const c) noexcept {
      for (; b != e; ++b) {
         REQUIRE(c == *b);
      }
   };

   SECTION("puts the values into the buffer") {
      target << u8 << u16 << u32 << u64 << s8 << s16 << s32 << s64;
      target.write_into(&s[0], sizeof(s));
      test_range(&memory[0], &memory[u_size], 0x81);
      test_range(&memory[u_size], &memory[u_size + s_size], 0x41);
      test_range(&memory[u_size + s_size], &memory[a_size], 0x46);
   }

   SECTION("causes the current index to move by size of the value") {
      std::size_t expected = 0U;
      REQUIRE(expected == target.current());
      target << u8;
      expected += sizeof(u8);
      REQUIRE(expected == target.current());
      target << u16;
      expected += sizeof(u16);
      REQUIRE(expected == target.current());
      target << u32;
      expected += sizeof(u32);
      REQUIRE(expected == target.current());
      target << u64;
      expected += sizeof(u64);
      REQUIRE(expected == target.current());
      target << s8;
      expected += sizeof(s8);
      REQUIRE(expected == target.current());
      target << s16;
      expected += sizeof(s16);
      REQUIRE(expected == target.current());
      target << s32;
      expected += sizeof(s32);
      REQUIRE(expected == target.current());
      target << s64;
      expected += sizeof(s64);
      REQUIRE(expected == target.current());
      target.write_into(s, sizeof(s));
      expected += sizeof(s);
      REQUIRE(expected == target.current());
   }

   SECTION("causes the remaining to reduce by size of the value") {
      std::size_t expected = sizeof(memory);
      REQUIRE(expected == target.remaining());
      target << u8;
      expected -= sizeof(u8);
      REQUIRE(expected == target.remaining());
      target << u16;
      expected -= sizeof(u16);
      REQUIRE(expected == target.remaining());
      target << u32;
      expected -= sizeof(u32);
      REQUIRE(expected == target.remaining());
      target << u64;
      expected -= sizeof(u64);
      REQUIRE(expected == target.remaining());
      target << s8;
      expected -= sizeof(s8);
      REQUIRE(expected == target.remaining());
      target << s16;
      expected -= sizeof(s16);
      REQUIRE(expected == target.remaining());
      target << s32;
      expected -= sizeof(s32);
      REQUIRE(expected == target.remaining());
      target << s64;
      expected -= sizeof(s64);
      REQUIRE(expected == target.remaining());
      target.write_into(s, sizeof(s));
      expected -= sizeof(s);
      REQUIRE(expected == target.remaining());
   }

   SECTION("keeps index in place when directly referencing location") {
      target.skip(picker.pick(std::size_t{}, target.remaining() / 2U));
      std::size_t const expected = target.current();
      target.write_at(u8, expected + 1);
      REQUIRE(expected == target.current());
      target.write_into_at(s, sizeof(s), 3);
      REQUIRE(expected == target.current());
   }

   SECTION("can have index updated when specifically set") {
      target.skip(picker.pick(std::size_t{1}, target.remaining() - 1U));
      REQUIRE(target.set_current(0));
      REQUIRE(0U == target.current());
      REQUIRE(!target.set_current(picker.pick(target.size())));
      REQUIRE(0U == target.current());
   }

   SECTION("will throw system error when checked and no more memory is available") {
      REQUIRE_NOTHROW(checked(target << u8));
      target.skip(target.remaining());
      REQUIRE_THROWS_AS(checked(target << u8), std::system_error);
   }
}


TEST_CASE("Sharing a buffer", "[write_buffer, read_buffer]") {
   alignas(64) char memory[64];
   write_buffer<native_endian_encoding> target{&memory[0], sizeof(memory)};
   char s[8];
   std::uint8_t u8 = picker.pick<uint8_t>();
   std::uint16_t u16 = picker.pick<uint16_t>();
   std::uint32_t u32 = picker.pick<uint32_t>();
   std::uint64_t u64 = picker.pick<uint64_t>();
   std::int8_t s8 = picker.pick<int8_t>();
   std::int16_t s16 = picker.pick<int16_t>();
   std::int32_t s32 = picker.pick<int32_t>();
   std::int64_t s64 = picker.pick<int64_t>();

   std::memset(memory, 0, sizeof(memory));
   std::generate_n(s, sizeof(s), [] { return picker.pick<char>(); });
   REQUIRE(sizeof(memory) == target.size());

   SECTION("has a read buffer of current write position (when no other ops occur)") {
      target << u8 << u16 << u32 << u64 << s8 << s16 << s32 << s64;
      target.write_into(&s[0], sizeof(s));
      auto reader = target.read();
      REQUIRE(reader.size() == target.current());
   }

   SECTION("has a read buffer with already written contents") {
      target << u8 << u16 << u32 << u64 << s8 << s16 << s32 << s64;
      target.write_into(&s[0], sizeof(s));
      auto reader = target.read();
      std::uint8_t u8_actual = reader.read();
      std::uint16_t u16_actual = reader.read();
      std::uint32_t u32_actual = reader.read();
      std::uint64_t u64_actual = reader.read();
      std::int8_t s8_actual = reader.read();
      std::int16_t s16_actual = reader.read();
      std::int32_t s32_actual = reader.read();
      std::int64_t s64_actual = reader.read();
      char s_actual[sizeof(s)];
      reader.read_into(s_actual, sizeof(s_actual));
      REQUIRE(u8 == u8_actual);
      REQUIRE(u16 == u16_actual);
      REQUIRE(u32 == u32_actual);
      REQUIRE(u64 == u64_actual);
      REQUIRE(s8 == s8_actual);
      REQUIRE(s16 == s16_actual);
      REQUIRE(s32 == s32_actual);
      REQUIRE(s64 == s64_actual);
      REQUIRE(std::equal(&s[0], &s[sizeof(s)],
               &s_actual[0], &s_actual[sizeof(s_actual)]));
   }

   SECTION("has no effects on existing read buffer if more writes happen") {
      target << u8 << u16 << u32 << u64;
      auto reader = target.read();
      reader >> u8 >> u16 >> u32 >> u64;
      REQUIRE(0U == reader.remaining());
      target << s8 << s16 << s32 << s64;
      REQUIRE(0U == reader.remaining());
   }
}
