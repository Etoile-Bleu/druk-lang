add_executable(druk_bench
    lexer_bench.cpp
)

target_link_libraries(druk_bench PRIVATE
    druk-core
    benchmark::benchmark_main
)

add_executable(sum_bench
    sum_bench.cpp
)

target_link_libraries(sum_bench PRIVATE
    druk-core
    benchmark::benchmark_main
)
