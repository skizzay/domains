ADD_LIBRARY(domains_utils INTERFACE)
TARGET_SOURCES(domains_utils INTERFACE
   function_traits.hpp
   memory_resource.hpp
   optional.hpp
   parameter_pack.hpp
   string_view.hpp
   type_traits.hpp)
