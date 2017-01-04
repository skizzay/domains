#pragma once

#include <domains/utils/type_traits.hpp>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <type_traits>
#include <experimental/type_traits>

namespace domains {
namespace details_ {

template <class View>
struct buffer_impl {
   constexpr buffer_impl() noexcept : start_{nullptr}, size_{0} {
   }

   constexpr buffer_impl(void *const start, std::size_t const size) noexcept : start_{start},
                                                                               size_{size} {
   }

   constexpr buffer_impl(buffer_impl &&other) noexcept : start_{other.start_}, size_{other.size_} {
      other.start_ = nullptr;
      other.size_ = 0;
   }

   constexpr buffer_impl &operator=(buffer_impl &&other) noexcept {
      if (this != &other) {
         this->start_ = other.start_;
         this->size_ = other.size_;
         other.start_ = nullptr;
         other.size_ = 0;
      }
      return *this;
   }

   constexpr buffer_impl(const buffer_impl &other) noexcept : start_{other.start_},
                                                              size_{other.size_} {
   }

   constexpr buffer_impl &operator=(const buffer_impl &other) noexcept {
      if (this != &other) {
         this->start_ = other.start_;
         this->size_ = other.size_;
      }
      return *this;
   }

   constexpr std::size_t remaining(const std::size_t current) const noexcept {
      return (static_cast<const unsigned char *>(start_) + size_) - current;
   }

   constexpr std::size_t size() const noexcept {
      return size_;
   }

   constexpr std::error_code put(const uint8_t value, const std::size_t end,
                                 std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint8_t), end, current)) {
         uint8_t *bytes = as_array<uint8_t>(current);
         bytes[0] = value;
         advance(sizeof(uint8_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code put(const uint16_t value, const std::size_t end,
                                 std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint16_t), end, current)) {
         uint8_t *bytes = as_array<uint8_t>(current);
         const uint8_t *value_bytes = reinterpret_cast<const uint8_t *>(&value);
         bytes[0] = value_bytes[0];
         bytes[1] = value_bytes[1];
         advance(sizeof(uint16_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code put(const uint32_t value, const std::size_t end,
                                 std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint32_t), end, current)) {
         uint8_t *bytes = as_array<uint8_t>(current);
         const uint8_t *value_bytes = reinterpret_cast<const uint8_t *>(&value);
         bytes[0] = value_bytes[0];
         bytes[1] = value_bytes[1];
         bytes[2] = value_bytes[2];
         bytes[3] = value_bytes[3];
         advance(sizeof(uint32_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code put(const uint64_t value, const std::size_t end,
                                 std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint64_t), end, current)) {
         uint8_t *bytes = as_array<uint8_t>(current);
         const uint8_t *value_bytes = reinterpret_cast<const uint8_t *>(&value);
         bytes[0] = value_bytes[0];
         bytes[1] = value_bytes[1];
         bytes[2] = value_bytes[2];
         bytes[3] = value_bytes[3];
         bytes[4] = value_bytes[4];
         bytes[5] = value_bytes[5];
         bytes[6] = value_bytes[6];
         bytes[7] = value_bytes[7];
         advance(sizeof(uint64_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code put(const void *const value, const std::size_t n,
                                 const std::size_t end, std::size_t &current) noexcept {
      using std::memcpy;
      if (has_enough_bytes(n, end, current)) {
         memcpy(index_to_memory(current), value, n);
         advance(n, current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code read_into(uint8_t &value, const std::size_t end,
                                       std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint8_t), end, current)) {
         value = *as_array<uint8_t>(current);
         advance(sizeof(uint8_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code read_into(uint16_t &value, const std::size_t end,
                                       std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint16_t), end, current)) {
         const uint8_t *bytes = as_array<uint8_t>(current);
         value = (static_cast<uint16_t>(bytes[0]) << 8) | (static_cast<uint16_t>(bytes[1]) << 0);
         advance(sizeof(uint16_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code read_into(uint32_t &value, const std::size_t end,
                                       std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint32_t), end, current)) {
         const uint8_t *bytes = as_array<uint8_t>(current);
         value = (static_cast<uint32_t>(bytes[0]) << 24) | (static_cast<uint32_t>(bytes[1]) << 16) |
                 (static_cast<uint32_t>(bytes[2]) << 8) | (static_cast<uint32_t>(bytes[3]) << 0);
         advance(sizeof(uint32_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   constexpr std::error_code read_into(uint64_t &value, const std::size_t end,
                                       std::size_t &current) noexcept {
      // LIKELY
      if (has_enough_bytes(sizeof(uint64_t), end, current)) {
         const uint8_t *bytes = as_array<uint8_t>(current);
         value = (static_cast<uint64_t>(bytes[0]) << 56) | (static_cast<uint64_t>(bytes[1]) << 48) |
                 (static_cast<uint64_t>(bytes[2]) << 40) | (static_cast<uint64_t>(bytes[3]) << 32) |
                 (static_cast<uint64_t>(bytes[4]) << 24) | (static_cast<uint64_t>(bytes[5]) << 16) |
                 (static_cast<uint64_t>(bytes[6]) << 8) | (static_cast<uint64_t>(bytes[7]) << 0);
         advance(sizeof(uint64_t), current);
         return {};
      }
      return make_error_code(std::errc::value_too_large);
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, T> read() {
      T t;
      check_read(t);
      return t;
   }

   template <class T>
   constexpr void check_read(T &value, const std::size_t end, std::size_t &current) {
      std::error_code ec = this->read_into(value, end, current);
      if (ec) {
         throw std::system_error{ec};
      }
   }

   template <class T>
   constexpr void check_put(const T value, const std::size_t end, std::size_t &current) {
      std::error_code ec = this->put(value, end, current);
      if (ec) {
         throw std::system_error{ec};
      }
   }

private:
   constexpr void *index_to_memory(const std::size_t index) const noexcept {
      return static_cast<void *>(&static_cast<unsigned char *>(start_)[index]);
   }

   template <class T>
   constexpr T *as_array(const std::size_t index) const noexcept {
      return &static_cast<T *>(start_)[index];
   }

   constexpr static bool has_enough_bytes(const std::size_t num_bytes, const std::size_t end,
                                          const std::size_t current) noexcept {
      return (current + num_bytes) <= end;
   }

   constexpr static void advance(const std::size_t num_bytes, std::size_t &current) noexcept {
      current += num_bytes;
   }

   void *start_;
   std::size_t size_;
};
}

struct read_buffer : private details_::buffer_impl<read_buffer> {
   using details_::buffer_impl<read_buffer>::buffer_impl;
   using details_::buffer_impl<read_buffer>::size;

   constexpr std::size_t available_to_read() const noexcept {
      return size() - read_index;
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, T> read() {
      T value;
      check_read(value, size(), read_index);
      return value;
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, std::error_code> read_into(T &value) noexcept {
      return this->read_into(value, size(), read_index);
   }

private:
   using details_::buffer_impl<read_buffer>::read_into;

   std::size_t read_index = 0;
};

struct write_buffer : private details_::buffer_impl<write_buffer> {
   using details_::buffer_impl<write_buffer>::buffer_impl;
   using details_::buffer_impl<write_buffer>::size;

   constexpr std::size_t available_to_write() const noexcept {
      return size() - write_index;
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, std::error_code> put(const T value) noexcept {
      return this->put(value, size(), write_index);
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, void> put_or_throw(const T value) {
      this->check_put(value, size(), write_index);
   }

private:
   using details_::buffer_impl<write_buffer>::put;

   std::size_t write_index = 0;
};

struct read_write_buffer : private details_::buffer_impl<read_write_buffer> {
   using details_::buffer_impl<read_write_buffer>::buffer_impl;
   using details_::buffer_impl<read_write_buffer>::size;

   constexpr std::size_t available_to_read() const noexcept {
      return write_index - read_index;
   }

   constexpr std::size_t available_to_write() const noexcept {
      return size() - write_index;
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, T> read() {
      T value;
      check_read(value, write_index, read_index);
      return value;
   }

   template <class T>
   constexpr std::enable_if_t<is_unsigned_v<T>, std::error_code> read_into(T &value) noexcept {
      return this->read_into(value, write_index, read_index);
   }

private:
   using details_::buffer_impl<read_write_buffer>::put;
   using details_::buffer_impl<read_write_buffer>::read_into;

   std::size_t read_index = 0;
   std::size_t write_index = 0;
};

template <class T>
constexpr read_buffer &operator>>(read_buffer &source, T &value) {
   value = source.read<T>();
   return source;
}

template <class T>
constexpr read_write_buffer &operator>>(read_write_buffer &source, T &value) {
   value = source.read<T>();
   return source;
}

template <class T>
constexpr write_buffer &operator<<(write_buffer &sink, const T value) {
   sink.put(value);
   return sink;
}
}
