add_executable(druk_bench
    main_bench.cpp
)

target_link_libraries(druk_bench PRIVATE
    druk-core
    benchmark::benchmark_main
)
