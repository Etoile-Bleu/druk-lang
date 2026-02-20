include(GoogleTest)

# ─── Shared include path for test helpers ─────────────────────────────────────
set(TEST_HELPERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ─── 1. Lexer tests ───────────────────────────────────────────────────────────
add_executable(druk_lexer_tests
    unit/lexer/test_lexer_tokens.cpp
    unit/lexer/test_lexer_keywords.cpp
    unit/lexer/test_lexer_numbers.cpp
    unit/lexer/test_lexer_strings.cpp
    unit/lexer/test_lexer_errors.cpp
)
target_include_directories(druk_lexer_tests PRIVATE ${TEST_HELPERS_DIR})
target_link_libraries(druk_lexer_tests PRIVATE
    druk_lexer
    druk_util
    GTest::gtest_main
)
gtest_discover_tests(druk_lexer_tests)

# ─── 2. Parser tests ──────────────────────────────────────────────────────────
add_executable(druk_parser_tests
    unit/parser/test_parser_expressions.cpp
    unit/parser/test_match_lambda.cpp
    unit/parser/test_parser_statements.cpp
    unit/parser/test_parser_control_flow.cpp
    unit/parser/test_parser_functions.cpp
    unit/parser/test_parser_collections.cpp
    unit/parser/test_parser_errors.cpp
)
target_include_directories(druk_parser_tests PRIVATE ${TEST_HELPERS_DIR})
target_link_libraries(druk_parser_tests PRIVATE
    druk_parser
    druk_lexer
    druk_util
    GTest::gtest_main
)
gtest_discover_tests(druk_parser_tests)

# ─── 3. Semantic tests ────────────────────────────────────────────────────────
add_executable(druk_semantic_tests
    unit/semantic/test_type_checker.cpp
    unit/semantic/test_scope_resolution.cpp
    unit/semantic/test_semantic_errors.cpp
)
target_include_directories(druk_semantic_tests PRIVATE ${TEST_HELPERS_DIR})
target_link_libraries(druk_semantic_tests PRIVATE
    druk_parser
    druk_lexer
    druk_semantic
    druk_util
    GTest::gtest_main
)
gtest_discover_tests(druk_semantic_tests)

# ─── 4. Runtime / Value tests ─────────────────────────────────────────────────
add_executable(druk_runtime_tests
    unit/runtime/test_value_system.cpp
)
target_include_directories(druk_runtime_tests PRIVATE ${TEST_HELPERS_DIR})
target_link_libraries(druk_runtime_tests PRIVATE
    druk_runtime
    druk_util
    GTest::gtest_main
)
gtest_discover_tests(druk_runtime_tests)

# ─── 5. Integration tests ─────────────────────────────────────────────────────
add_executable(druk_integration_tests
    integration/test_pipeline.cpp
)
target_include_directories(druk_integration_tests PRIVATE ${TEST_HELPERS_DIR})
target_link_libraries(druk_integration_tests PRIVATE
    druk_parser
    druk_lexer
    druk_semantic
    druk_util
    GTest::gtest_main
)
gtest_discover_tests(druk_integration_tests)
