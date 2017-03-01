#pragma once

#include <domains/utils/type_traits.hpp>
#include <cstdlib>
#include <stdexcept>

namespace domains {

// TODO: Pull out of header and place in source file.
inline namespace v1 {
inline void abort(std::exception const &) noexcept {
   std::abort();
}
}

template <class Parser, void (*OnException)(std::exception const &)=abort>
class protocol {
   Parser &parser;

public:
   explicit protocol(Parser &p) :
      parser(p)
   {
   }

   void process_buffer(void const *const data, std::size_t num_bytes) noexcept(noexcept(OnException(std::declval<std::exception const &>()))) {
      try {
         auto message_type = parser.get_message_type(data, num_bytes);
         parser.parse(message_type, data, num_bytes);
      }
      catch (std::exception const &e) {
         OnException(e);
      }
      catch (...) {
         std::abort();
      }
   }
};
}
