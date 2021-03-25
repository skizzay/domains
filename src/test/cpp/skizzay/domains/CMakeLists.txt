ADD_EXECUTABLE(domains_tests
   concepts.cpp
   main.cpp
   sequence.cpp
   aggregate/entity.cpp
   event_source/commit.cpp
   event_source/event.cpp
   event_source/event_stream.cpp
   event_source/inmemory/event_store.cpp
)
TARGET_INCLUDE_DIRECTORIES(domains_tests PRIVATE
   ${PROJECT_SOURCE_DIR}/src/main/cpp
   ${CONAN_INCLUDE_DIRS_CATCH}
)
TARGET_COMPILE_OPTIONS(domains_tests PRIVATE -fcoroutines -fconcepts-diagnostics-depth=5)
TARGET_LINK_OPTIONS(domains_tests PRIVATE -fcoroutines)
TARGET_LINK_LIBRARIES(domains_tests PRIVATE
   ${CONAN_LIBS_DIRS_KERCHOW}
   ${CONAN_LIBS_KERCHOW}
   ${CONAN_LIBS_DIRS_CATCH}
   ${CONAN_LIBS_CATCH}
)

ADD_TEST(event_source_tests
   COMMAND $<TARGET_FILE:domains_tests>
)

ADD_TEST(aggregate_tests
   $<TARGET_FILE:domains_tests> "[aggregate]"
)