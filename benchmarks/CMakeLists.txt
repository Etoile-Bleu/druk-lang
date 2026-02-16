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

add_executable(druk_compare_jit_bench
    compare_jit_bench.cpp
)

target_link_libraries(druk_compare_jit_bench PRIVATE
    druk-core
    benchmark::benchmark
)

add_executable(druk_lexer_perf_bench
    lexer_perf_bench.cpp
)

target_link_libraries(druk_lexer_perf_bench PRIVATE
    druk-core
    benchmark::benchmark_main
)
