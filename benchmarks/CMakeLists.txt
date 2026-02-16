add_executable(druk_bench
    lexer_bench.cpp
)

target_link_libraries(druk_bench PRIVATE
    druk-core
    benchmark::benchmark_main
)

add_executable(druk_compare_bench
    compare_bench.cpp
)

target_link_libraries(druk_compare_bench PRIVATE
    druk-core
    benchmark::benchmark_main
)
