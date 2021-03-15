ADD_LIBRARY(domains_event_source INTERFACE)
TARGET_SOURCES(domains_event_source INTERFACE
      commit.h
      concepts.h
      concurrency_collision.h
      event.h
      event_stream.h
      generator.h
      sequenced.h
      inmemory/event_store.h
   )