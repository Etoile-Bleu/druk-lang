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
if(NOT EMSCRIPTEN)
    find_package(ICU REQUIRED COMPONENTS uc data)
    message(STATUS "Found ICU ${ICU_VERSION}")
    
    # LLVM for JIT compilation - using LLVM 22.1.0-rc3
    set(LLVM_DIR "C:/dev/clang+llvm-22.1.0-rc3-x86_64-pc-windows-msvc/lib/cmake/llvm" CACHE PATH "LLVM CMake directory")
    find_package(LLVM CONFIG REQUIRED)
    if(LLVM_FOUND)
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
        message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
        
        include_directories(${LLVM_INCLUDE_DIRS})
        separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
        add_definitions(${LLVM_DEFINITIONS_LIST})
        
        llvm_map_components_to_libnames(llvm_libs 
            core support native orcjit executionengine
            target x86codegen x86asmparser passes)
        
        set(DRUK_HAVE_LLVM ON)
        add_compile_definitions(DRUK_HAVE_LLVM)
        add_compile_definitions(LLVM_VERSION_STRING="${LLVM_VERSION_STRING}")
        
        # Windows DIA SDK path for LLVM (must be before add_library)
        if(WIN32)
            link_directories("C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/DIA SDK/lib/amd64")
        endif()
    else()
        message(WARNING "LLVM not found. JIT compilation will be disabled.")
        set(DRUK_HAVE_LLVM OFF)
    endif()
endif()

include(FetchContent)

if(NOT EMSCRIPTEN)
    # GoogleTest
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)

    # GoogleBenchmark - Disabled on Windows due to regex issues
    # FetchContent_Declare(
    #     benchmark
    #     GIT_REPOSITORY https://github.com/google/benchmark.git
    #     GIT_TAG v1.8.3
    # )
    # set(BENCHMARK_ENABLE_TESTING OFF)
    # set(BENCHMARK_ENABLE_WERROR OFF)
    # FetchContent_MakeAvailable(benchmark)
endif()

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
    src/semantic/types.cpp
    src/semantic/table.cpp
    src/semantic/core/driver.cpp
    src/semantic/core/decl.cpp
    src/semantic/core/stmt.cpp
    src/semantic/core/expr.cpp
    src/codegen/chunk.cpp
    src/codegen/generator.cpp
    src/codegen/gen_expr.cpp
    src/codegen/gen_stmt.cpp
    src/codegen/gen_collections.cpp
    src/codegen/jit_runtime.cpp
    src/vm/vm.cpp
    src/druk_c_api.cpp
)

if(DRUK_HAVE_LLVM)
    target_sources(druk-core PRIVATE
        src/codegen/llvm_backend.cpp
        src/codegen/llvm_jit.cpp
    )
endif()

target_include_directories(druk-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(NOT EMSCRIPTEN)
    target_link_libraries(druk-core PUBLIC
        ICU::uc
        ICU::data
    )
    
    if(DRUK_HAVE_LLVM)
        llvm_map_components_to_libnames(LLVM_LIBRARIES 
            Core Support OrcJIT ExecutionEngine
            Target X86CodeGen X86AsmParser X86Desc X86Info
            Passes Analysis TransformUtils ScalarOpts InstCombine
            MC MCParser Object RuntimeDyld JITLink
            TargetParser AsmParser BitWriter IRReader)
        
        message(STATUS "LLVM libraries: ${LLVM_LIBRARIES}")
        target_link_libraries(druk-core PUBLIC ${LLVM_LIBRARIES})
        
        # Windows-specific libraries required by LLVM
        if(WIN32)
            target_link_libraries(druk-core PUBLIC ws2_32 version diaguids)
        endif()
    endif()
endif()

if(NOT MSVC)
    target_compile_options(druk-core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()


# Main Executable
add_executable(druk src/main.cpp)
target_link_libraries(druk PRIVATE druk-core)

# Microbenchmark (no Google Benchmark dependency)
add_executable(druk_compare_microbench benchmarks/compare_microbench.cpp)
target_link_libraries(druk_compare_microbench PRIVATE druk-core)

# Stub Executable for compiled programs
add_executable(druk-stub src/druk_stub.cpp)
target_link_libraries(druk-stub PRIVATE druk-core)
set_target_properties(druk-stub PROPERTIES OUTPUT_NAME "druk-stub")

# WASM target (Emscripten)
if(EMSCRIPTEN)
    add_executable(druk-wasm src/wasm/druk_wasm.cpp)
    target_link_libraries(druk-wasm PRIVATE druk-core)
    set_target_properties(druk-wasm PROPERTIES OUTPUT_NAME "druk_wasm")
    set_target_properties(druk-wasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_SOURCE_DIR}/web/public/wasm")
    target_link_options(druk-wasm PRIVATE
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=DrukWasm"
        "-sEXPORTED_FUNCTIONS=['_druk_wasm_run','_druk_wasm_free']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString']"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sENVIRONMENT=web")
endif()

# Testing
if(NOT EMSCRIPTEN)
    enable_testing()
    add_subdirectory(tests)
    # Temporarily disable benchmarks due to regex issues on Windows
    # add_subdirectory(benchmarks)
endif()
