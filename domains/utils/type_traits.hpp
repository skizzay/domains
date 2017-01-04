#pragma once

// clang-format off
#if __has_include(<experimental/type_traits>)
// clang-format on
#include <experimental/type_traits>

namespace domains {
inline namespace v1 { using std::experimental::is_unsigned_v; }
}

#else

namespace domains {
inline namespace v2 { using std::is_unsigned_v; }
}

#endif
