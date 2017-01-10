#pragma once

#include <domains/utils/type_traits.hpp>
#include <climits>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <type_traits>
#include <experimental/type_traits>

namespace domains {

struct encode_big_endian {
   static constexpr void put(const uint8_t value, uint8_t *const bytes) {
      bytes[0] = value;
   }

   static constexpr void put(const uint16_t value, uint8_t *const bytes) {
      bytes[0] = value >> (1 * CHAR_BIT);
      bytes[1] = value & 0xFF;
   }

   static constexpr void put(const uint32_t value, uint8_t *const bytes) {
      bytes[0] = value >> (3 * CHAR_BIT);
      bytes[1] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[2] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[3] = value & 0xFF;
   }

   static constexpr void put(const uint64_t value, uint8_t *const bytes) {
      bytes[0] = value >> (7 * CHAR_BIT);
      bytes[1] = (value >> (6 * CHAR_BIT)) & 0xFF;
      bytes[2] = (value >> (5 * CHAR_BIT)) & 0xFF;
      bytes[3] = (value >> (4 * CHAR_BIT)) & 0xFF;
      bytes[4] = (value >> (3 * CHAR_BIT)) & 0xFF;
      bytes[5] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[6] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[7] = value & 0xFF;
   }

   static constexpr void read(uint8_t &value, uint8_t const *const bytes) {
      value = bytes[0];
   }

   static constexpr void read(uint16_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint16_t>(bytes[0]) << (1 * CHAR_BIT)) |
              (static_cast<uint16_t>(bytes[1]) << (0 * CHAR_BIT));
   }

   static constexpr void read(uint32_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint32_t>(bytes[0]) << (3 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[1]) << (2 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[2]) << (1 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[3]) << (0 * CHAR_BIT));
   }

   static constexpr void read(uint64_t &value, uint8_t const *const bytes) {
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

struct encode_little_endian {
   static constexpr void put(const uint8_t value, uint8_t *const bytes) {
      bytes[0] = value;
   }

   static constexpr void put(const uint16_t value, uint8_t *const bytes) {
      bytes[1] = value >> (1 * CHAR_BIT);
      bytes[0] = value & 0xFF;
   }

   static constexpr void put(const uint32_t value, uint8_t *const bytes) {
      bytes[3] = value >> (3 * CHAR_BIT);
      bytes[2] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[1] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[0] = value & 0xFF;
   }

   static constexpr void put(const uint64_t value, uint8_t *const bytes) {
      bytes[7] = value >> (7 * CHAR_BIT);
      bytes[6] = (value >> (6 * CHAR_BIT)) & 0xFF;
      bytes[5] = (value >> (5 * CHAR_BIT)) & 0xFF;
      bytes[4] = (value >> (4 * CHAR_BIT)) & 0xFF;
      bytes[3] = (value >> (3 * CHAR_BIT)) & 0xFF;
      bytes[2] = (value >> (2 * CHAR_BIT)) & 0xFF;
      bytes[1] = (value >> (1 * CHAR_BIT)) & 0xFF;
      bytes[0] = value & 0xFF;
   }

   static constexpr void read(uint8_t &value, uint8_t const *const bytes) {
      value = bytes[0];
   }

   static constexpr void read(uint16_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint16_t>(bytes[1]) << (1 * CHAR_BIT)) |
              (static_cast<uint16_t>(bytes[0]) << (0 * CHAR_BIT));
   }

   static constexpr void read(uint32_t &value, uint8_t const *const bytes) {
      value = (static_cast<uint32_t>(bytes[3]) << (3 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[2]) << (2 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[1]) << (1 * CHAR_BIT)) |
              (static_cast<uint32_t>(bytes[0]) << (0 * CHAR_BIT));
   }

   static constexpr void read(uint64_t &value, uint8_t const *const bytes) {
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

struct encode_native_endian {
   template <class T>
   static constexpr void put(const T value, uint8_t *const bytes) {
      using std::memcpy;
      memcpy(bytes, &value, sizeof(T));
   }

   template <class T>
   static constexpr void read(T &value, uint8_t const *const bytes) {
      using std::memcpy;
      memcpy(&value, bytes, sizeof(T));
   }
};

namespace details_ {

template <typename Encoding>
struct buffer_impl {
   constexpr buffer_impl(void *const data, const std::size_t size) noexcept : start_{data},
                                                                              size_{size},
                                                                              index_{0} {
   }

   constexpr buffer_impl() noexcept : buffer_impl{nullptr, 0} {
   }

   constexpr buffer_impl(const buffer_impl &other) noexcept : start_{other.start_},
                                                              size_{other.size_},
                                                              index_{other.index_} {
   }

   constexpr buffer_impl(buffer_impl &&other) noexcept : start_{other.start_},
                                                         size_{other.size_},
                                                         index_{other.index_} {
      other.start_ = nullptr;
      other.size_ = 0;
      other.index_ = 0;
   }

   constexpr buffer_impl &operator=(const buffer_impl &other) noexcept {
      if (this != &other) {
         start_ = other.start_;
         size_ = other.size_;
         index_ = other.index_;
      }
      return *this;
   }

   constexpr buffer_impl &operator=(buffer_impl &&other) noexcept {
      if (this != &other) {
         start_ = other.start_;
         size_ = other.size_;
         index_ = other.index_;
         other.start_ = nullptr;
         other.size_ = 0;
         other.index_ = 0;
      }
      return *this;
   }

   constexpr void *index_to_memory(const std::size_t index) const noexcept {
      return static_cast<void *>(&static_cast<unsigned char *>(start_)[index]);
   }

   constexpr std::size_t current() const noexcept {
      return index_;
   }

   constexpr std::size_t size() const noexcept {
      return size_;
   }

   constexpr std::size_t remaining() const noexcept {
      return size() - current();
   }

   std::error_code seek_to(const std::size_t index) noexcept {
      // LIKELY
      if (index < size()) {
         index_ = index;
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr void *data() const noexcept {
      return index_to_memory(0);
   }

   template <typename UnsignedInteger>
   std::enable_if_t<is_unsigned_v<UnsignedInteger>, std::error_code>
   read_into(UnsignedInteger &value, const std::size_t current) const noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(UnsignedInteger), current)) {
         Encoding::read(value, as_array<uint8_t>(current));
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   template <typename UnsignedInteger>
   std::enable_if_t<is_unsigned_v<UnsignedInteger>, std::error_code>
   put_from(const UnsignedInteger value, const std::size_t current) const noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(UnsignedInteger), current)) {
         Encoding::put(value, as_array<uint8_t>(current));
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   template <class T>
   constexpr void check_read(T &value, const std::size_t current) {
      std::error_code ec = this->read_into(value, current);
      if (ec) {
         throw std::system_error{ec};
      }
   }

   std::error_code put(const void *const value, const std::size_t n,
                       const std::size_t current) noexcept {
      using std::memcpy;
      // LIKELY
      if (has_enough_bytes(n, current)) {
         memcpy(index_to_memory(current), value, n);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   template <class T>
   constexpr void check_put(const T value, std::size_t &current) {
      std::error_code ec = this->put(value, current);
      if (ec) {
         throw std::system_error{ec};
      }
   }

   constexpr void unchecked_advance(const std::size_t num_bytes) noexcept {
      index_ += num_bytes;
   }

   std::error_code skip(const std::size_t num_bytes) noexcept {
      return checked_advance(num_bytes);
   }

private:
   template <class T>
   constexpr T *as_array(const std::size_t index) const noexcept {
      return static_cast<T *>(index_to_memory(index));
   }

   constexpr bool has_enough_bytes(const std::size_t num_bytes, const std::size_t index) const
       noexcept {
      const std::size_t new_index = index + num_bytes;
      return (new_index <= size()) && (new_index > index);
   }

   std::error_code checked_advance(const std::size_t num_bytes) noexcept {
      // LIKELY
      if (has_enough_bytes(num_bytes, current())) {
         unchecked_advance(num_bytes);
         return {};
      }
      return std::make_error_code(std::errc::value_too_large);
   }

   void *start_;
   std::size_t size_;
   std::size_t index_;
};
}

template <class Encoding>
class read_buffer : private details_::buffer_impl<Encoding> {
   class mutable_reader {
      read_buffer &buffer;

   public:
      explicit constexpr mutable_reader(read_buffer &b) noexcept : buffer{b} {
      }

      template <class UnsignedInteger>
      constexpr std::enable_if_t<is_unsigned_v<UnsignedInteger>, UnsignedInteger> to() {
         UnsignedInteger value{};
         buffer.check_read(value, buffer.current());
         buffer.unchecked_advance(sizeof(UnsignedInteger));
         return value;
      }

      template <class SignedInteger>
      constexpr std::enable_if_t<is_signed_v<SignedInteger>, SignedInteger> to() {
         return this->to<typename std::make_unsigned<SignedInteger>::type>();
      }

      template <class T>
      constexpr operator T() {
         return this->to<T>();
      }
   };

   class const_reader {
      read_buffer const &buffer;
      std::size_t const index;

   public:
      explicit constexpr const_reader(read_buffer const &b, std::size_t const n) noexcept
          : buffer{b},
            index{n} {
      }

      template <class UnsignedInteger>
      operator std::enable_if_t<is_unsigned_v<UnsignedInteger>, UnsignedInteger>() const {
         UnsignedInteger value;
         buffer.check_read(value, index);
         return value;
      }
   };

public:
   using details_::buffer_impl<Encoding>::buffer_impl;
   using details_::buffer_impl<Encoding>::size;
   using details_::buffer_impl<Encoding>::remaining;
   using details_::buffer_impl<Encoding>::current;
   using details_::buffer_impl<Encoding>::skip;
   using details_::buffer_impl<Encoding>::seek_to;

   constexpr mutable_reader read_next() {
      return mutable_reader(*this);
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, std::error_code> read_next(T &value) noexcept {
      auto ec = this->read_into(value, current());
      // LIKELY
      if (!ec) {
         this->unchecked_advance(sizeof(T));
      }
      return ec;
   }

   template <class T>
   constexpr const_reader read_at(const std::size_t index) const {
      return {*this, index};
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, std::error_code>
   read_at(T &value, const std::size_t index) const noexcept {
      return this->read_into(value, index);
   }
};

template <class Encoding>
struct write_buffer : private details_::buffer_impl<Encoding> {
   using details_::buffer_impl<Encoding>::buffer_impl;
   using details_::buffer_impl<Encoding>::size;
   using details_::buffer_impl<Encoding>::remaining;
   using details_::buffer_impl<Encoding>::current;
   using details_::buffer_impl<Encoding>::skip;
   using details_::buffer_impl<Encoding>::seek_to;

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, std::error_code> put_next(const T value) noexcept {
      auto ec = this->put_from(value, current());
      // LIKELY
      if (!ec) {
         this->unchecked_advance(sizeof(T));
      }
      return ec;
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, void> put_next_or_throw(const T value) {
      this->check_put(value, current());
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, std::error_code>
   put_at(const T value, const std::size_t index) noexcept {
      return this->put(value, index);
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, void> put_at_or_throw(const T value,
                                                                      const std::size_t index) {
      this->check_put(value, index);
   }

   constexpr read_buffer<Encoding> read() const noexcept {
      return {this->data(), this->current()};
   }
};

template <class Encoding, class T>
constexpr read_buffer<Encoding> &operator>>(read_buffer<Encoding> &source, T &value) {
   value = source.read_next();
   return source;
}

template <class Encoding, class T>
constexpr write_buffer<Encoding> &operator<<(write_buffer<Encoding> &sink, const T value) {
   sink.put_next_or_throw(value);
   return sink;
}
}
