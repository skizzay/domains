#pragma once

#if __has_include(<experimental/optional>)
#  include <experimental/optional>

namespace domains {
inline namespace latst_v1 {
   using std::experimental::optional;
   using std::experimental::in_place_t;
   using std::experimental::in_place;
   using std::experimental::nullopt_t;
   using std::experimental::nullopt;
   using std::experimental::bad_optional_access;
   using std::experimental::make_optional;
}
}

#elif __has_include(<optional>)
#  include <optional>

namespace domains {
inline namespace latst_v2 {
   using std::optional;
   using std::in_place_t;
   using std::in_place;
   using std::nullopt_t;
   using std::nullopt;
   using std::bad_optional_access;
   using std::make_optional;
}
}

#endif
