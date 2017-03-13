ADD_LIBRARY(domains_messaging INTERFACE)
TARGET_SOURCES(domains_messaging INTERFACE
   buffer.hpp
   dispatcher.hpp
   encoding.hpp
   messaging_traits.hpp)
