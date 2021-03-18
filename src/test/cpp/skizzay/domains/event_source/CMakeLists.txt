ADD_EXECUTABLE(event_source_tests
   commit.cpp
   concepts.cpp
   event.cpp
   event_stream.cpp
   sequence.cpp
   inmemory/event_store.cpp
   main.cpp)
TARGET_INCLUDE_DIRECTORIES(event_source_tests PRIVATE
   ${PROJECT_SOURCE_DIR}/src/main/cpp
   ${CONAN_INCLUDE_DIRS_CATCH})
TARGET_COMPILE_OPTIONS(event_source_tests PRIVATE -fcoroutines -fconcepts-diagnostics-depth=5)
TARGET_LINK_OPTIONS(event_source_tests PRIVATE -fcoroutines)
TARGET_LINK_LIBRARIES(event_source_tests PRIVATE
   ${CONAN_LIBS_DIRS_KERCHOW}
   ${CONAN_LIBS_KERCHOW}
   ${CONAN_LIBS_DIRS_CATCH}
   ${CONAN_LIBS_CATCH})
