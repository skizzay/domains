#pragma once

#include <climits>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <type_traits>

namespace domains {

struct big_endian_encoding {
   static constexpr void encode(const uint8_t value, uint8_t *const bytes) {
      bytes[0] = value;
   }

   static constexpr void encode(const uint16_t value, uint8_t *const bytes) {
      bytes[0] = value >> (1 * CHAR_BIT);
      bytes[1] = value & 0xFF;
   }

   static constexpr void encode(const uint32_t value, uint8_t *const bytes) {
      bytes[0] = value >> (3 * CHAR_BIT);
      bytes[1] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[2] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[3] = value & 0xFF;
   }

   static constexpr void encode(const uint64_t value, uint8_t *const bytes) {
      bytes[0] = value >> (7 * CHAR_BIT);
      bytes[1] = (value >> (6 * CHAR_BIT)) & 0xFF;
      bytes[2] = (value >> (5 * CHAR_BIT)) & 0xFF;
      bytes[3] = (value >> (4 * CHAR_BIT)) & 0xFF;
      bytes[4] = (value >> (3 * CHAR_BIT)) & 0xFF;
      bytes[5] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[6] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[7] = value & 0xFF;
   }

   static constexpr void decode(uint8_t &value, uint8_t const *const bytes) {
      value = bytes[0];
   }

   static constexpr void decode(uint16_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint16_t>(bytes[0]) << (1 * CHAR_BIT)) |
              (static_cast<uint16_t>(bytes[1]) << (0 * CHAR_BIT));
   }

   static constexpr void decode(uint32_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint32_t>(bytes[0]) << (3 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[1]) << (2 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[2]) << (1 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[3]) << (0 * CHAR_BIT));
   }

   static constexpr void decode(uint64_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint64_t>(bytes[0]) << (7 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[1]) << (6 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[2]) << (5 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[3]) << (4 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[4]) << (3 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[5]) << (2 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[6]) << (1 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[7]) << (0 * CHAR_BIT));
   }
};

struct little_endian_encoding {
   static constexpr void encode(const uint8_t value, uint8_t *const bytes) {
      bytes[0] = value;
   }

   static constexpr void encode(const uint16_t value, uint8_t *const bytes) {
      bytes[1] = value >> (1 * CHAR_BIT);
      bytes[0] = value & 0xFF;
   }

   static constexpr void encode(const uint32_t value, uint8_t *const bytes) {
      bytes[3] = value >> (3 * CHAR_BIT);
      bytes[2] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[1] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[0] = value & 0xFF;
   }

   static constexpr void encode(const uint64_t value, uint8_t *const bytes) {
      bytes[7] = value >> (7 * CHAR_BIT);
      bytes[6] = (value >> (6 * CHAR_BIT)) & 0xFF;
      bytes[5] = (value >> (5 * CHAR_BIT)) & 0xFF;
      bytes[4] = (value >> (4 * CHAR_BIT)) & 0xFF;
      bytes[3] = (value >> (3 * CHAR_BIT)) & 0xFF;
      bytes[2] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[1] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[0] = value & 0xFF;
   }

   static constexpr void decode(uint8_t &value, uint8_t const *const bytes) {
      value = bytes[0];
   }

   static constexpr void decode(uint16_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint16_t>(bytes[1]) << (1 * CHAR_BIT)) |
              (static_cast<uint16_t>(bytes[0]) << (0 * CHAR_BIT));
   }

   static constexpr void decode(uint32_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint32_t>(bytes[3]) << (3 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[2]) << (2 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[1]) << (1 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[0]) << (0 * CHAR_BIT));
   }

   static constexpr void decode(uint64_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint64_t>(bytes[7]) << (7 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[6]) << (6 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[5]) << (5 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[4]) << (4 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[3]) << (3 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[2]) << (2 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[1]) << (1 * CHAR_BIT)) |
              (static_cast<uint64_t>(bytes[0]) << (0 * CHAR_BIT));
   }
};

struct native_endian_encoding {
   template <class T>
   static constexpr void encode(T const value, uint8_t *const bytes) {
      using std::memcpy;
      memcpy(bytes, &value, sizeof(T));
   }

   template <class T>
   static constexpr void decode(T &value, uint8_t const *const bytes) {
      using std::memcpy;
      memcpy(&value, bytes, sizeof(T));
   }
};
}
