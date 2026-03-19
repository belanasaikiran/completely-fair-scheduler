cmake_minimum_required(VERSION 3.20)
project(cfs_scheduler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/catch2.git
    GIT_TAG        v3.7.1
)

FetchContent_MakeAvailable(Catch2)

# main library
add_library(cfs_lib STATIC
    src/core.cpp
)

target_include_directories(cfs_lib PUBLIC src)

# Test executable
add_executable(cfs_tests
    tests/job_test.cpp
)
target_include_directories(cfs_tests PRIVATE
    cfs_lib
    Catch2::Catch2WithMain # provides main() automatically
)

# Register with CTest
include(CTest)
include(Catch)
catch_discover_tests(cfs_tests)
