#pragma once

#if __has_include(<string_view>)
#  include <string_view>

namespace domains {
inline namespace latest_v2 {
   using std::basic_string_view;
   using std::string_view;
   using std::wstring_view;
   namespace literals {
      namespace string_view_literals = std::literals::string_view_literals;
   }
}
}

#elif __has_include(<experimental/string_view>)
#  include <experimental/string_view>

namespace domains {
inline namespace latest_v1 {
   using std::experimental::basic_string_view;
   using std::experimental::string_view;
   using std::experimental::wstring_view;
   namespace literals {
      namespace string_view_literals = std::literals::string_view_literals;
   }
}
}

#endif
