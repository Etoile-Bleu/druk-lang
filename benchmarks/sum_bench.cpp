#include <benchmark/benchmark.h>
#include "druk/lexer/lexer.hpp"
#include "druk/parser/core/parser.hpp"
#include "druk/interpreter/interpreter.hpp"
#include <string>

// Simple C++ sum for comparison
static void BM_CPP_Sum(benchmark::State &state) {
  for (auto _ : state) {
    int sum = 0;
    for (int i = 0; i < 1000; ++i) {
      sum += i;
    }
    benchmark::DoNotOptimize(sum);
  }
  state.SetItemsProcessed(int64_t(state.iterations()) * 1000);
}
BENCHMARK(BM_CPP_Sum);

// Druk sum - parsing and interpreting
static void BM_Druk_Sum(benchmark::State &state) {
  // Druk code to sum numbers from 0 to 999
  // Using correct Dzongkha/Tibetan keywords:
  // - གྲངས་ཀ (grang-ka) = number type declaration
  // - རིམ་པ (rim-pa) = loop statement
  // - English equivalent: 
  //   number sum = 0;
  //   number i = 0;
  //   loop (i < 1000) { sum = sum + i; i = i + 1; }
  std::string source = R"(
གྲངས་ཀ sum = 0;
གྲངས་ཀ i = 0;
རིམ་པ (i < 1000) {
  sum = sum + i;
  i = i + 1;
}
  )";

  // Parse once outside the benchmark loop
  druk::ArenaAllocator arena;
  druk::StringInterner interner(arena);
  druk::ErrorReporter errors;
  
  druk::Parser parser(source, arena, interner, errors);
  auto program = parser.parse();

  for (auto _ : state) {
    // Only execute the code in the benchmark loop
    druk::Interpreter interpreter(source);
    interpreter.execute(program);
    
    benchmark::DoNotOptimize(program);
  }
  
  state.SetItemsProcessed(int64_t(state.iterations()) * 1000);
}
BENCHMARK(BM_Druk_Sum);

BENCHMARK_MAIN();
