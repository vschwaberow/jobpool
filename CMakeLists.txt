cmake_minimum_required(VERSION 3.14)
project(ComplexTasksWithDNS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Option to choose between static and dynamic library
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

set(CPM_DOWNLOAD_VERSION 0.40.2)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake...")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()
include(${CPM_DOWNLOAD_LOCATION})

# Fetch Asio (standalone) using CPM
CPMAddPackage(
  NAME asio
  VERSION 1.28.0
  GITHUB_REPOSITORY chriskohlhoff/asio
  GIT_TAG asio-1-28-0
  DOWNLOAD_ONLY YES
)

if(asio_ADDED)
  add_library(asio INTERFACE)
  target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
  target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
endif()

CPMAddPackage(
  NAME GoogleTest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.12.1
  VERSION 1.12.1
  OPTIONS
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt ON"
)

CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.14.1
)

set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Create the library
add_library(complex_tasks_lib
  src/job_pool.cc
)

target_include_directories(complex_tasks_lib PUBLIC ${INCLUDE_DIR})
target_link_libraries(complex_tasks_lib PUBLIC
  asio
  spdlog::spdlog
)

# Create the example executable
add_executable(complex_tasks_with_dns
  src/complex_tasks_with_dns.cc
)

target_link_libraries(complex_tasks_with_dns PRIVATE complex_tasks_lib)

# Unit tests
add_executable(unit_tests
  tests/job_pool_test.cc
  tests/dns_resolver_test.cc
)

target_link_libraries(unit_tests PRIVATE
  complex_tasks_lib
  gtest
  gtest_main
)

enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)

# Installation
install(TARGETS complex_tasks_lib complex_tasks_with_dns
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
install(FILES ${INCLUDE_DIR}/job_pool.h ${INCLUDE_DIR}/dns_resolver.h DESTINATION include)

# Compiler options
target_compile_options(complex_tasks_lib PRIVATE
  $<$<CONFIG:Debug>:-g -O0>
  $<$<CONFIG:Release>:-O3>
)

target_compile_options(complex_tasks_with_dns PRIVATE
  $<$<CONFIG:Debug>:-g -O0>
  $<$<CONFIG:Release>:-O3>
)

target_compile_options(unit_tests PRIVATE
  $<$<CONFIG:Debug>:-g -O0>
  $<$<CONFIG:Release>:-O3>
)

if(UNIX)
  target_link_libraries(complex_tasks_lib PUBLIC pthread)
endif()
