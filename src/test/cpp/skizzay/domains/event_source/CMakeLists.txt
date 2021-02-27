ADD_EXECUTABLE(event_source_tests
   commit.cpp
   concepts.cpp
   event.cpp
   sequence.cpp
   main.cpp)
TARGET_INCLUDE_DIRECTORIES(event_source_tests PRIVATE
   ${PROJECT_SOURCE_DIR}/src/main/cpp
   ${CONAN_INCLUDE_DIRS_CATCH})
TARGET_LINK_LIBRARIES(event_source_tests PRIVATE
   ${CONAN_LIBS_DIRS_KERCHOW}
   ${CONAN_LIBS_KERCHOW}
   ${CONAN_LIBS_DIRS_CATCH}
   ${CONAN_LIBS_CATCH})
