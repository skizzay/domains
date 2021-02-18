ADD_LIBRARY(domains_event_source INTERFACE)
TARGET_SOURCES(domains_event_source INTERFACE
      commit.h
      concepts.h
      event.h
      sequenced.h
   )