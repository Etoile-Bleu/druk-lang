include(GoogleTest)

add_executable(druk_tests
    main_test.cpp
    allocator_test.cpp
    token_test.cpp
    interner_test.cpp
    lexer_test.cpp
    parser/parser_test.cpp
)

target_link_libraries(druk_tests PRIVATE
    druk-core
    GTest::gtest_main
)

gtest_discover_tests(druk_tests)
