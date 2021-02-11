#pragma once
#include <domains/messaging/buffer.hpp>
#include <domains/messaging/decoder.hpp>
#include <domains/messaging/dispatcher.hpp>
#include <domains/messaging/encoding.hpp>
#include <domains/messaging/translator.hpp>

#include <domains/messaging/buffer.hpp>
#include <domains/utils/byte.hpp>

#include <experimental/string_view>

#include <vector>

namespace {
template <class T, class = void>
struct decode_helper_ {
   static std::error_code do_decode(domains::read_buffer<domains::native_endian_encoding>, T &) {
      return {};
   }
};

template <class T>
struct decode_helper_<T, std::void_t<decltype(&T::value)>> {
   static std::error_code
   do_decode(domains::read_buffer<domains::native_endian_encoding> encoded_value, T &decoded) {
      if (encoded_value.remaining() > 0) {
         uint16_t num_bytes = encoded_value.read();
         decoded.value.resize(num_bytes);
         encoded_value.read_into(&decoded.value[0], num_bytes);
      }
      return encoded_value.error();
   }
};

class data {
   std::vector<domains::byte> bytes;

public:
   explicit data(domains::read_buffer<domains::native_endian_encoding> buffer)
      : bytes(buffer.remaining()) {
      buffer.read_into(bytes.data(), bytes.size());
   }

   domains::read_buffer<domains::native_endian_encoding> read() const noexcept {
      return {bytes.data(), bytes.size()};
   }
};

enum class data_type : uint8_t { A, B, C, D, E, G };

struct A {
   virtual ~A() noexcept = default;
   virtual std::string name() const = 0;
};

struct B final : A {
   virtual std::string name() const override {
      return "B::" + value;
   }
   std::string value;
};

struct C : A {
   virtual std::string name() const override {
      return "C::" + value;
   }
   std::string value;
};

struct D final {};

struct E final : C {
   virtual std::string name() const override {
      return "E::" + value;
   }
   std::string value;
};

struct G : A {
   virtual std::string name() const override {
      return "G::" + value;
   }
   std::string value;
};

struct factory final {
   size_t a_count = 0;
   size_t b_count = 0;
   size_t c_count = 0;
   size_t d_count = 0;
   size_t e_count = 0;

   auto make_single_domain_dispatcher() noexcept {
      return domains::single_dispatcher(
          [this](A const &) noexcept->std::error_code {
             ++a_count;
             return {};
          },
          [this](B const &) noexcept->std::error_code {
             ++b_count;
             return {};
          },
          [this](C const &) noexcept->std::error_code {
             ++c_count;
             return {};
          },
          [this](D const &) noexcept->std::error_code {
             ++d_count;
             return {};
          },
          [this](E const &) noexcept->std::error_code {
             ++e_count;
             return {};
          },
          [](std::experimental::string_view s) noexcept->size_t { return s.size(); },
          [](int i) noexcept->int { return i; });
   }

   auto make_single_primitive_dispatcher() noexcept {
      return domains::single_dispatcher([](std::experimental::string_view s) noexcept
                                                 ->size_t { return s.size(); },
                                             [](int i) noexcept->int { return i; },
                                             [this](A const &) noexcept->std::error_code {
                                                ++a_count;
                                                return {};
                                             },
                                             [this](D const &) noexcept->std::error_code {
                                                ++d_count;
                                                return {};
                                             });
   }

   auto make_catchall_dispatcher() noexcept {
      return [](auto const &) noexcept->std::error_code {
         return {};
      };
   }

   auto make_decode_dispatcher() noexcept {
      return [](auto encoded_value, auto &decoded_value) {
         return decode_helper_<std::decay_t<decltype(decoded_value)>>::do_decode(encoded_value,
                                                                                 decoded_value);
      };
   }

   auto make_translator() noexcept {
      return domains::parsing_translator(make_decode_dispatcher(),
                                              domains::compact_multi_type_provider<B, C, D, G>{});
   }

   auto make_data_router() noexcept {
      return [](data const &d, auto &&dispatcher) noexcept->std::error_code {
         auto buffer = d.read();
         data_type type = buffer.read();
         switch (type) {
         case data_type::B:
            return dispatcher.template decode_and_dispatch<B>(buffer);

         case data_type::C:
            return dispatcher.template decode_and_dispatch<C>(buffer);

         case data_type::D:
            return dispatcher.template decode_and_dispatch<D>(buffer);

         case data_type::G:
            return dispatcher.template decode_and_dispatch<G>(buffer);

         default:
            return make_error_code(std::errc::not_supported);
         }
      };
   }

   auto make_decoder() noexcept {
      return domains::decoder(make_data_router(), make_translator());
   }
};
}
