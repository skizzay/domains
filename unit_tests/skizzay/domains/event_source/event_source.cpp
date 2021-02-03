#include <domains/event_source/event_source.hpp>
#include <domains/messaging/buffer.hpp>
#include <domains/messaging/decoder.hpp>
#include <kerchow/kerchow.h>
#if __has_include(<catch / catch.hpp>)
#include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#include <catch.hpp>
#endif

using namespace domains;
using kerchow::picker;

TEST_CASE("Null event source", "[event_source, null_event_source]") {
   null_event_source_t<int> target;

   SECTION("does not return an error when saving an event") {
      REQUIRE(!target.put(picker.pick<uint32_t>(), picker.pick<char>()));
   }

   SECTION("num_events always returns 0") {
      REQUIRE(target.num_events(picker.pick<uint32_t>()) == std::uint32_t{0U});
   }
}

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
   int a;
};

struct B final : A {
   char b;
};

struct C {
   std::string s;
};

enum class data_type : uint8_t { A, B, C };

class fake_entity final {
   std::uint16_t id_;
   int a_;
   std::string c_;

public:
   explicit fake_entity(std::uint16_t const id) noexcept : id_{id}, a_{0}, c_{} {
   }

   void on(A const &a) noexcept {
      this->a_ = a.a;
   }

   void on(C const &c) noexcept {
      this->c_ = c.s;
   }

   std::uint16_t id() const noexcept {
      return id_;
   }

   unsigned version() const noexcept {
      return 0U;
   }

   int a() const noexcept {
      return a_;
   }

   std::string const &c() const noexcept {
      return c_;
   }
};

auto get_event_dispatcher(fake_entity &entity) noexcept {
   return [&entity](auto const &event) noexcept->std::error_code {
      entity.on(event);
      return {};
   };
}

class fake_encoder final {
   byte memory[64];

public:
   data operator()(A const &a) noexcept {
      write_buffer<native_endian_encoding> buffer(&memory[0], sizeof(memory));
      buffer << data_type::A << a.a;
      return data(buffer.read());
   }

   data operator()(B const &b) noexcept {
      write_buffer<native_endian_encoding> buffer(&memory[0], sizeof(memory));
      buffer << data_type::B << b.a << b.b;
      return data(buffer.read());
   }

   data operator()(C const &c) noexcept {
      write_buffer<native_endian_encoding> buffer(&memory[0], sizeof(memory));
      buffer << data_type::C << static_cast<std::uint16_t>(c.s.size());
      buffer.write_into(&c.s[0], c.s.size());
      return data(buffer.read());
   }
};
}

TEST_CASE("Buffered event source", "[event_source, memory_store]") {
   auto router = [](data const &d, auto dispatcher) noexcept {
      auto buffer = d.read();
      data_type type = buffer.read();
      switch (type) {
      case data_type::A:
         return dispatcher.template decode_and_dispatch<A>(buffer.subbuffer());
      case data_type::B:
         return dispatcher.template decode_and_dispatch<B>(buffer.subbuffer());
      case data_type::C:
         return dispatcher.template decode_and_dispatch<C>(buffer.subbuffer());
      default:
         return make_error_code(std::errc::not_supported);
      }
   };
   auto decode_dispatcher = single_dispatcher(
       [](read_buffer<native_endian_encoding> buff, A & a) noexcept {
          buff >> a.a;
          return buff.error();
       },
       [](read_buffer<native_endian_encoding> buff, B & b) noexcept {
          buff >> b.a >> b.b;
          return buff.error();
       },
       [](read_buffer<native_endian_encoding> buff, C & c) noexcept {
          std::uint16_t n = buff.read();
          c.s.resize(n);
          buff.read_into(&c.s[0], n);
          return buff.error();
       });

   event_source target = event_source(
       decoder(std::move(router), parsing_translator(std::move(decode_dispatcher), compact_multi_type_provider<A, B, C>{})),
       fake_encoder{}, memory_store<std::uint16_t, data>{});
   std::uint16_t const id = picker.pick<std::uint16_t>();
   fake_entity entity(id);

   SECTION("builds entity from saved events.") {
      target.put(id, A{1});
      target.build(entity);
      REQUIRE(entity.a() == 1);
   }

   SECTION("knows how many events were saved for an entity.") {
      B b;
      b.a = 1;
      b.b = 'b';
      target.put(id, b);
      b.a = 2;
      target.put(id, b);
      target.put(id, A{1});
      target.put(id, C{"hello"});
      REQUIRE(target.num_events(id) == 4);
   }
}
