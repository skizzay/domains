ADD_EXECUTABLE(messaging_tests
   main.cpp
   buffer.cpp
   dispatcher.cpp
   encoding.cpp)
TARGET_INCLUDE_DIRECTORIES(messaging_tests PRIVATE
   ${PROJECT_SOURCE_DIR}
   ${CONAN_INCLUDE_DIRS_CATCH})
TARGET_LINK_LIBRARIES(messaging_tests PRIVATE
   ${CONAN_LIBS_DIRS_KERCHOW}
   ${CONAN_LIBS_KERCHOW}
   ${CONAN_LIBS_DIRS_CATCH}
   ${CONAN_LIBS_CATCH})
