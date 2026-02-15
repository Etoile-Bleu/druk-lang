cmake_minimum_required(VERSION 3.27)
project(druk-lang VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W3 /permissive- /volatile:iso /Zc:inline)
    # add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=address,undefined")
endif()

# Dependencies
find_package(ICU REQUIRED COMPONENTS uc data)

include(FetchContent)

# GoogleTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# GoogleBenchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_WERROR OFF)
FetchContent_MakeAvailable(benchmark)

# Core Library
add_library(druk-core STATIC
    src/common/allocator.cpp
    src/common/error.cpp
    src/lexer/token.cpp
    src/lexer/unicode.cpp
    src/lexer/lexer.cpp
    src/parser/core/parser.cpp
    src/parser/core/decl.cpp
    src/parser/core/stmt.cpp
    src/parser/core/expr.cpp
    src/parser/core/recovery.cpp
)

target_include_directories(druk-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(druk-core PUBLIC
    ICU::uc
    ICU::data
)

if(NOT MSVC)
    target_compile_options(druk-core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()


# Main Executable
add_executable(druk src/main.cpp)
target_link_libraries(druk PRIVATE druk-core)

# Testing
enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)
