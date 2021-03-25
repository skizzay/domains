ADD_LIBRARY(domains_event_source INTERFACE)
TARGET_SOURCES(domains_event_source INTERFACE
      commit.h
      concurrency_collision.h
      event_store.h
      event_stream.h
      event.h
      generator.h
      inmemory/event_store.h
   )