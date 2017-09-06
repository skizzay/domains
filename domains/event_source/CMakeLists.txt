ADD_LIBRARY(domains_event_source INTERFACE)
TARGET_SOURCES(domains_event_source INTERFACE
      event.hpp
      event_range.hpp
      event_source.hpp
      event_store.hpp
      event_stream.hpp
   )
