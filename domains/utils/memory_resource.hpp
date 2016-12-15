#pragma once

#if __has_include(<memory_resource>)
#  include <memory_resource>

namespace domains {
inline namespace latest_v2 {
   using std::pmr::memory_resource;
   using std::pmr::polymorphic_allocator;
   using std::pmr::resource_adaptor;
   using std::pmr::new_delete_resource;
   using std::pmr::null_memory_resource;
   using std::pmr::set_default_resource;
   using std::pmr::get_default_resource;
}
}

#elif __has_include(<experimental/memory_resource>)
#  include <experimental/memory_resource>

namespace domains {
inline namespace latest_v1 {
   using std::experimental::pmr::memory_resource;
   using std::experimental::pmr::polymorphic_allocator;
   using std::experimental::pmr::resource_adaptor;
   using std::experimental::pmr::new_delete_resource;
   using std::experimental::pmr::null_memory_resource;
   using std::experimental::pmr::set_default_resource;
   using std::experimental::pmr::get_default_resource;
}
}

#endif
