#include <domains/messaging/protocol.hpp>
#include <iostream>

class simple_parser {
public:
   using message_type = uint16_t;
   using encoding_type = domains::encode_native_endian;

   constexpr message_type get_message_type(void const *const data, std::size_t num_bytes) const noexcept {
      return 0;
   }

   constexpr message_type get_message_type(domains::read_buffer<> buffer) const noexcept {
      return 0;
   }

   void parse(message_type, void const *const data, std::size_t num_bytes) noexcept {
      message.assign(static_cast<char const *const>(data), num_bytes);
   }

   std::string const &parse(domains::read_buffer<> buffer) noexcept {
      return message.assign(static_cast<char const *const>(data), num_bytes);
   }

private:
   std::string message;
};

int main(int argc, char **argv) {
   char const data[] = "Hello World";
   domains::protocol<simple_parser> protocol;
   protocol.process_buffer(data, sizeof(data) - 1U);
   return 0;
}
