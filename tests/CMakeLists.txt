include(GoogleTest)

add_executable(druk_tests
    main_test.cpp
    allocator_test.cpp
    token_test.cpp
    interner_test.cpp
    lexer_test.cpp
    parser/parser_test.cpp
    semantic/table_test.cpp
    semantic/analyze_test.cpp
    codegen/chunk_test.cpp
    codegen/generator_test.cpp
    vm/vm_test.cpp
)

target_link_libraries(druk_tests PRIVATE
    druk-core
    GTest::gtest_main
)

gtest_discover_tests(druk_tests)
