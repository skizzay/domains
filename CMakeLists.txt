project(DomainsUnitTest)
cmake_minimum_required(VERSION 2.8.12)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS fsm utilz kerchow catch2)
include(res/Catch)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fstrict-aliasing -Werror")

add_library(skizzay_domains INTERFACE)
target_include_directories(skizzay_domains INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(skizzay_domains INTERFACE CONAN_PKG::fsm CONAN_PKG::utilz)

enable_testing()
add_subdirectory(unit_tests)
#add_subdirectory(cqrs)
