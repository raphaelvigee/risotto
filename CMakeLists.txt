cmake_minimum_required(VERSION 3.10)
project(risottov2)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message("Using ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif ()

message("Adding build types...")
SET(CMAKE_CXX_FLAGS_COVERAGE
        "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C++ compiler during coverage builds."
        FORCE)
SET(CMAKE_C_FLAGS_COVERAGE
        "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C compiler during coverage builds."
        FORCE)
SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used for linking binaries during coverage builds."
        FORCE)
SET(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used by the shared libraries linker during coverage builds."
        FORCE)
MARK_AS_ADVANCED(
        CMAKE_CXX_FLAGS_COVERAGE
        CMAKE_C_FLAGS_COVERAGE
        CMAKE_EXE_LINKER_FLAGS_COVERAGE
        CMAKE_SHARED_LINKER_FLAGS_COVERAGE)


if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    MESSAGE("Building with coverage flags")

    INCLUDE(CodeCoverage.cmake)
endif ()

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("lib/googletest")

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/googlebenchmark REQUIRED)

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include" "${gmock_SOURCE_DIR}/include")
endif ()

file(GLOB_RECURSE LIB_WISE_ENUM_SRC RELATIVE ${CMAKE_SOURCE_DIR} "lib/wise_enum/wise_enum.h")
file(GLOB_RECURSE LIB_VEC_SRC RELATIVE ${CMAKE_SOURCE_DIR} "lib/vec/src/*")
file(GLOB_RECURSE LIB_CXXOPTS_SRC RELATIVE ${CMAKE_SOURCE_DIR} "lib/cxxopts/include/cxxopts.hpp")

file(GLOB_RECURSE RISOTTO_LIB_SRC RELATIVE ${CMAKE_SOURCE_DIR} "src/lib/*.c" "src/lib/*.cpp" "src/lib/*.h")

set(CMD_COMMON ${LIB_CXXOPTS_SRC})
set(LIB_COMMON ${LIB_WISE_ENUM_SRC} ${LIB_VEC_SRC})

file(GLOB_RECURSE CMD_SRC RELATIVE ${CMAKE_SOURCE_DIR} "src/cmd/cmd.cpp" )

# LIBRARY

add_library(risotto_lib STATIC ${LIB_COMMON} ${RISOTTO_LIB_SRC})
target_include_directories(risotto_lib PUBLIC ${CMAKE_SOURCE_DIR}/src)

# DEBUG LIBRARY

add_library(risotto_lib_debug STATIC ${LIB_COMMON} ${RISOTTO_LIB_SRC})
target_compile_definitions(risotto_lib_debug PUBLIC RISOTTO_DEBUG_LIB)
target_compile_definitions(risotto_lib_debug PUBLIC DEBUG_TRACE_EXECUTION)
target_compile_definitions(risotto_lib_debug PUBLIC BENCHMARK_TIMINGS)
target_include_directories(risotto_lib_debug PUBLIC ${CMAKE_SOURCE_DIR}/src)

# CMD

add_executable(rst ${CMD_COMMON} ${CMD_SRC})
target_link_libraries(rst risotto_lib)

# DEBUG CMD

add_executable(rst-debug ${CMD_COMMON} ${CMD_SRC})
target_link_libraries(rst-debug risotto_lib_debug)

# BENCHMARK

file(GLOB_RECURSE BENCHMARKS_SRC RELATIVE ${CMAKE_SOURCE_DIR} "benchmarks/*.c" "benchmarks/*.cpp" "benchmarks/*.h")

add_executable(ben ${BENCHMARKS_SRC})
target_link_libraries(ben risotto_lib)
target_link_libraries(ben benchmark::benchmark)

# TESTING

enable_testing()

if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    SETUP_TARGET_FOR_COVERAGE(
            gtest_coverage  # Name for custom target.
            tests         # Name of the test driver executable that runs the tests.
            # NOTE! This should always have a ZERO as exit code
            # otherwise the coverage generation will not complete.
            coverage            # Name of output directory.
    )
endif ()

file(GLOB_RECURSE TESTS_SRC RELATIVE ${CMAKE_SOURCE_DIR} "tests/*.c" "tests/*.cpp" "tests/*.h")

add_executable(tests ${TESTS_SRC})
target_link_libraries(tests risotto_lib_debug)
target_link_libraries(tests gtest_main)
