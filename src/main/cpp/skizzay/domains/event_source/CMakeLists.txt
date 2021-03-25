ADD_LIBRARY(domains_event_source INTERFACE)
TARGET_SOURCES(domains_event_source INTERFACE
      commit.h
      concepts.h
      concurrency_collision.h
      event_store.h
      event_stream.h
      event.h
      generator.h
      sequence.h
      inmemory/event_store.h
   )