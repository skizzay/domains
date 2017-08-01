ADD_LIBRARY(domains_utils INTERFACE)
TARGET_SOURCES(domains_utils INTERFACE
   any.hpp
   byte.hpp
   function_traits.hpp
   memory_resource.hpp
   null_mutex.hpp
   optional.hpp
   parameter_pack.hpp
   string_view.hpp
   type_provider.hpp
   type_traits.hpp)
