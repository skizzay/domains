#include "domains/messaging/protocol.hpp"
#if __has_include(<catch/catch.hpp>)
#  include <catch/catch.hpp>
#elif __has_include(<catch.hpp>)
#  include <catch.hpp>
#endif
#include <algorithm>

using namespace domains;

namespace {

class mock_parser {
public:
   using message_type = uint16_t;

   message_type id = 0U;
   bool is_good_sequence = false;

   message_type get_message_type(void const *const data, std::size_t num_bytes) const noexcept {
      invocation.resize(num_bytes);
      std::copy(static_cast<char const *>(data),
            static_cast<char const *>(data) + num_bytes,
            invocation.begin());
      return id;
   }

   void parse(message_type id, void const *const data, std::size_t num_bytes) noexcept {
      is_good_sequence = (id == this->id) &&
         std::equal(static_cast<char const *>(data), static_cast<char const *>(data) + num_bytes,
               invocation.begin(), invocation.end());
   }

private:
   mutable std::vector<char> invocation;
};

static bool exception_caught = false;
void track_thrown_exception(std::exception const &) {
   exception_caught = true;
}

}

TEST_CASE("Protocol invokes parser properly") {
   exception_caught = false;
   char const data[] = "Hello World";
   mock_parser parser;
   protocol<mock_parser, track_thrown_exception> target(parser);

   SECTION("Processing buffer gets message type and parses message") {
      target.process_buffer(data, sizeof(data) - 1U);

      REQUIRE(parser.is_good_sequence);
      REQUIRE(exception_caught == false);
   }
}
