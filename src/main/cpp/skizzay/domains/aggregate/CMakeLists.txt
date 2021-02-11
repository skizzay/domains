ADD_LIBRARY(domains_aggregate INTERFACE)
TARGET_SOURCES(domains_aggregate INTERFACE
   event_source.hpp
   event_store.hpp)
