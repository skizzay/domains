#pragma once

// clang-format off
#if __has_include(<experimental/type_traits>)
// clang-format on
#include <experimental/type_traits>

namespace domains {
inline namespace v1 {
using std::experimental::is_signed_v;
using std::experimental::is_unsigned_v;
using std::experimental::is_integral_v;
using std::experimental::is_nothrow_constructible_v;
using std::experimental::is_nothrow_move_constructible_v;
using std::experimental::is_same_v;
using std::experimental::is_base_of_v;
}
}

#else

namespace domains {
inline namespace v2 {
using std::is_signed_v;
using std::is_unsigned_v;
using std::is_integral_v;
using std::is_nothrow_constructible_v;
using std::is_nothrow_move_constructible_v;
using std::is_same_v;
using std::is_base_of_v;
}
}

#endif

namespace domains {
template <template <class...> class F, class... T>
concept bool as_concept = F<T...>::value;
}
