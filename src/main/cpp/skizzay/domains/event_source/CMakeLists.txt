ADD_LIBRARY(domains_event_source INTERFACE)
TARGET_SOURCES(domains_event_source INTERFACE
      commit.h
      concepts.h
      event.h
      event_stream.h
      sequenced.h
      inmemory/event_stream.h
   )