cmake_minimum_required(VERSION 3.27)
project(druk-lang VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DRUK_VERSION)
    set(DRUK_VERSION ${PROJECT_VERSION})
endif()
add_compile_definitions(DRUK_VERSION="${DRUK_VERSION}")

# Compiler flags
if(MSVC)
    add_compile_options(/W3 /permissive- /volatile:iso /Zc:inline /utf-8)
    # Disable noisy LLVM warnings and CRT warnings
    add_compile_options(/wd4244 /wd4267 /wd4146 /wd4291 /wd4996 /wd4018)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=address,undefined")
endif()

# Dependencies
if(NOT EMSCRIPTEN)
    find_package(ICU REQUIRED COMPONENTS uc data)
    message(STATUS "Found ICU ${ICU_VERSION}")
    
    # LLVM for JIT compilation
    if(WIN32)
        set(LLVM_DIR "C:/dev/clang+llvm-22.1.0-rc3-x86_64-pc-windows-msvc/lib/cmake/llvm" CACHE PATH "LLVM CMake directory")
    endif()
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
        
        # Disable LLVM warnings
        if(MSVC)
            add_compile_options(/wd4005)  # Macro redefinition
            add_compile_options(/wd4624)  # Implicitly deleted destructor
        endif()
        
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

# Common Utilities Library
add_library(druk_util STATIC
    src/util/arena_allocator.cpp
    src/util/error_handler.cpp
    src/util/error_formatter.cpp
    src/util/interner.cpp
    src/util/utf8.cpp
    src/util/update_checker.cpp
)
target_include_directories(druk_util PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Semantic Library
add_library(druk_semantic STATIC
    src/semantic/analyzer.cpp
    src/semantic/name_resolver.cpp
    src/semantic/tc_core.cpp
    src/semantic/tc_stmt_decl.cpp
    src/semantic/tc_stmt_flow.cpp
    src/semantic/tc_type.cpp
    src/semantic/tc_expr_simple.cpp
    src/semantic/tc_expr_complex.cpp
    src/semantic/symbol_table.cpp
    src/semantic/types.cpp
)
target_link_libraries(druk_semantic PUBLIC druk_util)
target_include_directories(druk_semantic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Lexer Library
add_library(druk_lexer STATIC
    src/lexer/core/lexer.cpp
    src/lexer/core/token.cpp
    src/lexer/core/unicode.cpp
    src/lexer/scanning/numeric.cpp
    src/lexer/scanning/string.cpp
    src/lexer/scanning/identifier.cpp
)
target_link_libraries(druk_lexer PUBLIC druk_util)
target_include_directories(druk_lexer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(NOT EMSCRIPTEN)
    target_link_libraries(druk_lexer PUBLIC
        ICU::uc
        ICU::data
    )
endif()

# Runtime Library (GC, Value System, Chunk)
add_library(druk_runtime STATIC
    src/codegen/core/chunk.cpp
    src/codegen/core/value.cpp
    src/codegen/core/obj.cpp
    src/gc/gc_heap_alloc.cpp
    src/gc/gc_heap_collect.cpp
    src/gc/gc_roots.cpp
    src/gc/gc_array.cpp
    src/gc/gc_string.cpp
    src/gc/gc_struct.cpp
)
target_link_libraries(druk_runtime PUBLIC druk_util)
target_include_directories(druk_runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Parser Library
add_library(druk_parser STATIC
    src/parser/core/parser.cpp
    src/parser/core/recovery.cpp
    src/parser/core/parser_type.cpp
    src/parser/ast/ast_stmt.cpp
    src/parser/ast/ast_expr.cpp
    src/parser/ast/type.cpp
    src/parser/stmt/basic.cpp
    src/parser/stmt/match_stmt.cpp
    src/parser/expr/lambda_expr.cpp
    src/parser/stmt/control.cpp
    src/parser/stmt/while.cpp
    src/parser/stmt/for.cpp
    src/parser/stmt/decl.cpp
    src/parser/expr/binary.cpp
    src/parser/expr/math.cpp
    src/parser/expr/unary.cpp
    src/parser/expr/postfix.cpp
    src/parser/expr/primary.cpp
    src/parser/expr/collections.cpp
)
target_link_libraries(druk_parser PUBLIC druk_lexer druk_semantic druk_runtime)
target_include_directories(druk_parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Core Library (Aggregator for now, will be split further)
add_library(druk-core STATIC
    src/codegen/core/code_generator_core.cpp
    src/codegen/core/code_generator_expr_binary.cpp
    src/codegen/core/code_generator_expr_simple.cpp
    src/codegen/core/code_generator_stmt_flow.cpp
    src/codegen/core/code_generator_stmt_loop.cpp
    src/codegen/core/code_generator_stmt_basic.cpp
    src/codegen/core/code_generator_func.cpp
    src/codegen/core/code_generator_call.cpp
    src/codegen/core/code_generator_array.cpp
    src/codegen/core/code_generator_match.cpp
    src/codegen/core/code_generator_lambda.cpp
    src/codegen/core/code_generator_stubs.cpp
    src/codegen/core/target_info.cpp
    src/codegen/core/abi_handler.cpp
    src/codegen/jit/runtime/rt_core.cpp
    src/codegen/jit/runtime/rt_value.cpp
    src/codegen/jit/runtime/rt_math.cpp
    src/codegen/jit/runtime/rt_compare.cpp
    src/codegen/jit/runtime/rt_globals.cpp
    src/codegen/jit/runtime/rt_array.cpp
    src/codegen/jit/runtime/rt_struct.cpp
    src/codegen/jit/runtime/rt_io.cpp
    src/codegen/jit/runtime/rt_call.cpp
    src/codegen/jit/runtime/rt_string.cpp
    src/codegen/jit/runtime/rt_null.cpp
    src/ir/ir_basic_block.cpp
    src/ir/ir_builder.cpp
    src/ir/ir_builder_array.cpp
    src/ir/ir_function.cpp
    src/ir/ir_instruction_ops.cpp
    src/ir/ir_instruction_memory.cpp
    src/ir/ir_instruction_control.cpp
    src/ir/ir_instruction_arrays.cpp
    src/ir/ir_module.cpp
    src/ir/ir_type.cpp
    src/ir/ir_value.cpp
    src/codegen/llvm/runtime.cpp
    src/codegen/llvm/backend_ir_constants.cpp
    src/codegen/llvm/backend_ir_compile_main.cpp
    src/codegen/llvm/backend_ir_compile_func.cpp
    src/codegen/llvm/backend_ir_emit_obj.cpp
    src/codegen/llvm/backend_ir_instructions.cpp
    src/codegen/llvm/backend_ir_binary_ops.cpp
    src/codegen/llvm/backend_ir_array_ops.cpp
    src/codegen/llvm/backend_ir_array_build.cpp
    src/codegen/llvm/backend_ir_array_index.cpp
    src/codegen/llvm/backend_ir_array_len.cpp
    src/codegen/llvm/backend_ir_memory_ops.cpp
    src/codegen/llvm/backend_ir_print.cpp
    src/codegen/llvm/backend_ir_string_ops.cpp
    src/codegen/llvm/backend_ir_unary_ops.cpp
    src/codegen/llvm/backend_ir_control_flow.cpp
    src/codegen/llvm/backend_ir_dynamic_call.cpp
    src/codegen/llvm/backend_ir_null.cpp
    src/vm/vm.cpp
    src/druk_c_api.cpp
)
target_link_libraries(druk-core PUBLIC druk_parser druk_semantic druk_runtime)

if(DRUK_HAVE_LLVM)
    target_sources(druk-core PRIVATE
        src/codegen/llvm/llvm_backend_init.cpp
        src/codegen/llvm/llvm_backend_symbols.cpp
        src/codegen/llvm/llvm_backend_symbols2.cpp
        src/codegen/llvm/llvm_backend_utils.cpp
        src/codegen/llvm/llvm_codegen_core.cpp
        src/codegen/llvm/llvm_codegen_find_linker.cpp
        src/codegen/llvm/llvm_codegen_link_exec.cpp
        src/codegen/llvm/llvm_jit.cpp
        src/codegen/llvm/llvm_ir_builder.cpp
    )
endif()

target_include_directories(druk-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(NOT EMSCRIPTEN)
    if(DRUK_HAVE_LLVM)
        llvm_map_components_to_libnames(LLVM_LIBRARIES 
            Core Support OrcJIT ExecutionEngine
            Target X86CodeGen X86AsmParser X86Desc X86Info
            Passes Analysis TransformUtils ScalarOpts InstCombine
            MC MCParser Object RuntimeDyld JITLink
            TargetParser AsmParser BitWriter IRReader)
        
        target_link_libraries(druk-core PUBLIC ${LLVM_LIBRARIES})
        
        if(WIN32)
            target_link_libraries(druk-core PUBLIC ws2_32 version diaguids)
        endif()
    endif()
endif()

if(NOT MSVC)
    target_compile_options(druk_util PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    target_compile_options(druk_lexer PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    target_compile_options(druk-core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()


# Main Executable
add_executable(druk src/main.cpp)
target_link_libraries(druk PRIVATE druk-core)

# Microbenchmark
# add_executable(druk_compare_microbench benchmarks/compare_microbench.cpp)
# target_link_libraries(druk_compare_microbench PRIVATE druk-core)

# Stub Executable
add_executable(druk-stub src/druk_stub.cpp)
target_link_libraries(druk-stub PRIVATE druk-core)
set_target_properties(druk-stub PROPERTIES OUTPUT_NAME "druk-stub")

# WASM target
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
endif()
