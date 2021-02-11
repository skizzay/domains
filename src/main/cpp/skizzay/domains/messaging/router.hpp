#pragma once

#include <system_error>

namespace domains {
struct null_router_t final {
   template <class T, class U>
   constexpr std::error_code operator()(T const &&, U &&) const noexcept {
      return {};
   }
};
constexpr null_router_t null_router{};
}
