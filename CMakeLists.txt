CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
PROJECT(domains)

SET(CMAKE_EXPORT_COMPILE_COMMANDS 1)

IF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   SET(warnings "-Wall -Wextra -Werror")
ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
   SET(warnings "/W4 /WX /EHsc")
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -fconcepts -fstrict-aliasing ${warnings}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${warnings}")

SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
CONAN_BASIC_SETUP()

#STRING(REPLACE "-std=c++11" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

ADD_SUBDIRECTORY(domains/utils)
ADD_SUBDIRECTORY(domains/messaging)
ADD_SUBDIRECTORY(domains/aggregate)

IF(BUILD_TESTS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0 -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0 -fprofile-arcs -ftest-coverage")
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(unit_tests/domains/utils)
  ADD_SUBDIRECTORY(unit_tests/domains/messaging)
  ADD_SUBDIRECTORY(unit_tests/domains/aggregate)
  ADD_TEST(utils_tests bin/utils_tests)
  ADD_TEST(messaging_tests bin/messaging_tests)
  ADD_TEST(aggregate_tests bin/aggregate_tests)
ENDIF()
