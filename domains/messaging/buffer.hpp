#pragma once

#include <domains/messaging/encoding.hpp>
#include <domains/utils/type_traits.hpp>

#include <climits>
#include <cstdint>
#include <cstring>
#include <experimental/type_traits>
#include <system_error>

namespace domains {

namespace details_ {

struct buffer_impl {
   buffer_impl(void *const data, const std::size_t size) noexcept : data_{data},
                                                                    size_{size},
                                                                    index_{0},
                                                                    error_() {
      if ((size == 0U) || (data_ == nullptr)) {
         data_ = nullptr;
         size_ = 0U;
      }
   }

   buffer_impl() noexcept : buffer_impl{nullptr, 0} {
   }

   buffer_impl(const buffer_impl &other) noexcept : data_{other.data_},
                                                    size_{other.size_},
                                                    index_{other.index_},
                                                    error_{other.error_} {
   }

   buffer_impl(buffer_impl &&other) noexcept : data_{other.data_},
                                               size_{other.size_},
                                               index_{other.index_},
                                               error_{std::move(other.error_)} {
      other.data_ = nullptr;
      other.size_ = 0;
      other.index_ = 0;
   }

   buffer_impl &operator=(const buffer_impl &other) noexcept {
      if (this != &other) {
         data_ = other.data_;
         size_ = other.size_;
         index_ = other.index_;
         error_ = other.error_;
      }
      return *this;
   }

   buffer_impl &operator=(buffer_impl &&other) noexcept {
      if (this != &other) {
         data_ = other.data_;
         size_ = other.size_;
         index_ = other.index_;
         other.data_ = nullptr;
         other.size_ = 0;
         other.index_ = 0;
      }
      return *this;
   }

   void *index_to_memory(const std::size_t index) const noexcept {
      return static_cast<void *>(&static_cast<unsigned char *>(data_)[index]);
   }

   std::size_t current() const noexcept {
      return index_;
   }

   std::size_t size() const noexcept {
      return size_;
   }

   std::size_t remaining() const noexcept {
      return size() - current();
   }

   explicit operator bool() const noexcept {
      return remaining() != 0U;
   }

   void seek(std::ptrdiff_t const offset) noexcept {
      if (((offset > 0) && (current() + offset) < size()) ||
          ((offset < 0) && (current() > static_cast<std::size_t const>(std::abs(offset)))) ||
          (offset == 0)) {
         advance(offset);
      } else {
         error_ = make_error_code(std::errc::result_out_of_range);
      }
   }

   void *data() const noexcept {
      return index_to_memory(0);
   }

   void advance(std::ptrdiff_t const num_bytes) noexcept {
      index_ += num_bytes;
   }

   std::error_code error() const noexcept {
      return error_;
   }

   void clear_error() noexcept {
      error_ = {};
   }

   bool set_current(std::size_t const i) noexcept {
      // LIKELY
      if (i < size()) {
         index_ = i;
         return true;
      }
      return false;
   }

   void skip(std::size_t const num_bytes) noexcept {
      safe_action(num_bytes, current(), [=]() { advance(num_bytes); });
   }

   template <class T>
   T *as_array(std::size_t const index) const noexcept {
      return static_cast<T *>(index_to_memory(index));
   }

   bool has_enough_bytes(std::size_t const num_bytes, std::size_t const index) const noexcept {
      std::size_t const new_index = index + num_bytes;
      return (index < new_index) && (new_index <= size());
   }

   template <class F>
   void safe_action(std::size_t const num_bytes, std::size_t const index, F f) noexcept {
      // LIKELY
      if (!error_) {
         // LIKELY
         if (has_enough_bytes(num_bytes, index)) {
            f();
         } else {
            error_ = make_error_code(std::errc::not_enough_memory);
         }
      }
   }

   template <class F>
   void safe_action(std::size_t const num_bytes, std::size_t const index, F f) const noexcept {
      // LIKELY
      if (has_enough_bytes(num_bytes, index)) {
         f();
      }
   }

   void *data_;
   std::size_t size_;
   std::size_t index_;
   std::error_code error_;
};
}

template <class Encoding>
class read_buffer final : private details_::buffer_impl {
   class mutable_reader {
      read_buffer &buffer;

      template <class UnsignedInteger>
      std::enable_if_t<is_unsigned_v<UnsignedInteger>, UnsignedInteger> to() noexcept {
         UnsignedInteger value{};
         buffer.safe_action(sizeof(UnsignedInteger), buffer.current(), [&] {
            Encoding::decode(value, buffer.as_array<uint8_t const>(buffer.current()));
            buffer.advance(sizeof(UnsignedInteger));
         });
         return value;
      }

      template <class SignedInteger>
      std::enable_if_t<is_signed_v<SignedInteger>, SignedInteger> to() noexcept {
         return this->to<typename std::make_unsigned<SignedInteger>::type>();
      }

   public:
      mutable_reader(read_buffer &b) noexcept : buffer{b} {
      }

      template <class T>
      operator T() noexcept {
         return this->to<T>();
      }
   };

   class const_reader {
      read_buffer const &buffer;
      std::size_t const index;

   public:
      const_reader(read_buffer const &b, std::size_t const n) noexcept : buffer{b}, index{n} {
      }

      template <class T>
      operator T() const {
         T value{};
         buffer.safe_action(sizeof(T), index, [&] {
            Encoding::decode(value, buffer.as_array<uint8_t const>(index));
         });
         return value;
      }
   };

   friend class mutable_reader;
   friend class const_reader;

public:
   using details_::buffer_impl::buffer_impl;
   using details_::buffer_impl::size;
   using details_::buffer_impl::remaining;
   using details_::buffer_impl::current;
   using details_::buffer_impl::skip;
   using details_::buffer_impl::seek;
   using details_::buffer_impl::data;
   using details_::buffer_impl::set_current;
   using details_::buffer_impl::error;

   template <class T>
   read_buffer<Encoding> &operator>>(T &value) noexcept {
      value = read();
      return *this;
   }

   mutable_reader read() noexcept {
      return {*this};
   }

   const_reader read_at(std::size_t const index) const noexcept {
      return {*this, index};
   }

   void read_into(void *const destination, std::size_t const num_bytes) noexcept {
      safe_action(num_bytes, current(), [&] {
         std::memcpy(destination, index_to_memory(current()), num_bytes);
         advance(num_bytes);
      });
   }

   void read_into_at(void *const destination, std::size_t const num_bytes,
                     std::size_t const index) const noexcept {
      safe_action(num_bytes, index,
                  [&] { std::memcpy(destination, index_to_memory(index), num_bytes); });
   }
};

template <class Encoding>
class write_buffer : details_::buffer_impl {
public:
   using details_::buffer_impl::buffer_impl;
   using details_::buffer_impl::size;
   using details_::buffer_impl::remaining;
   using details_::buffer_impl::current;
   using details_::buffer_impl::skip;
   using details_::buffer_impl::seek;
   using details_::buffer_impl::data;
   using details_::buffer_impl::set_current;
   using details_::buffer_impl::error;

   template <class T>
   write_buffer<Encoding> &operator<<(T const value) noexcept {
      write(value);
      return *this;
   }

   template <class T>
   void write(T const value) noexcept {
      safe_action(sizeof(T), current(), [=] {
         Encoding::encode(value, as_array<uint8_t>(current()));
         advance(sizeof(T));
      });
   }

   void write_into(void const *const source, std::size_t const num_bytes) noexcept {
      safe_action(num_bytes, current(), [=] {
         std::memcpy(index_to_memory(current()), source, num_bytes);
         advance(num_bytes);
      });
   }

   template <class T>
   void write_at(T const value, std::size_t const index) noexcept {
      safe_action(sizeof(T), index, [=] { Encoding::encode(value, as_array<uint8_t>(index)); });
   }

   void write_into_at(void const *const source, std::size_t const num_bytes,
                      std::size_t const index) noexcept {
      safe_action(num_bytes, index, [=] {
         std::memcpy(index_to_memory(index), source, num_bytes);
      });
   }

   read_buffer<Encoding> read() const noexcept {
      return {index_to_memory(0U), current()};
   }
};


template <class Encoding, template <class> class Buffer>
constexpr Buffer<Encoding> &checked(Buffer<Encoding> &buffer) {
   if (buffer.error()) {
      throw std::system_error{buffer.error()};
   }
   return buffer;
}
}
