cmake_minimum_required(VERSION 3.14)
project(ComplexTasksWithDNS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type to Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Add compiler warnings
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include CPM.cmake
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

# Fetch Boost using CPM
CPMAddPackage(
  NAME Boost
  VERSION 1.85.0
  GITHUB_REPOSITORY "boostorg/boost"
  GIT_TAG "boost-1.85.0"
)

# Fetch Google Test using CPM
CPMAddPackage(
  NAME GoogleTest
  GITHUB_REPOSITORY google/googletest
  GIT_TAG release-1.12.1
  VERSION 1.12.1
  OPTIONS
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt ON"
)

# Fetch spdlog using CPM
CPMAddPackage(
  NAME spdlog
  GITHUB_REPOSITORY gabime/spdlog
  VERSION 1.14.1
)

# Main executable
add_executable(complex_tasks_with_dns
  src/job_pool.cc
  src/complex_tasks_with_dns.cc
)

target_include_directories(complex_tasks_with_dns PRIVATE include)
target_link_libraries(complex_tasks_with_dns PRIVATE
  Boost::system
  Boost::asio
  spdlog::spdlog
)

# Tests executable
add_executable(unit_tests
  tests/job_pool_test.cc
  tests/dns_resolver_test.cc
  src/job_pool.cc
)

target_include_directories(unit_tests PRIVATE include)
target_link_libraries(unit_tests PRIVATE
  gtest
  gtest_main
  Boost::system
  Boost::asio
  spdlog::spdlog
)

# Enable testing
enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)

# Install rules
install(TARGETS complex_tasks_with_dns DESTINATION bin)
install(FILES include/job_pool.h include/dns_resolver.h DESTINATION include)

# Debug and Release specific flags
target_compile_options(complex_tasks_with_dns PRIVATE
  $<$<CONFIG:Debug>:-g -O0 -Wno-strict-prototypes>
  $<$<CONFIG:Release>:-O3 -Wno-strict-prototypes>
)

target_compile_options(unit_tests PRIVATE
  $<$<CONFIG:Debug>:-g -O0 -Wno-strict-prototypes>
  $<$<CONFIG:Release>:-O3 -Wno-strict-prototypes>
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wno-strict-prototypes)
endif()
