#include "druk/lexer/lexer.hpp"
#include <benchmark/benchmark.h>
#include <string>
#include <vector>


static void BM_Lexer(benchmark::State &state) {
  druk::ArenaAllocator arena;
  druk::StringInterner interner(arena);
  druk::ErrorReporter errors;

  // Create a large source string
  std::string source;
  for (int i = 0; i < 1000; ++i) {
    source +=
        "ལས་ཀ func_name(arg1, arg2) { གལ་ཏེ (x > 10) { འབྲི(\"hello\"); } } ";
  }
  // Length around 60-70 chars * 1000 = 60-70KB.

  for (auto _ : state) {
    druk::Lexer lexer(source, arena, interner, errors);
    while (true) {
      druk::Token token = lexer.next();
      if (token.kind == druk::TokenKind::EndOfFile)
        break;
    }

    // Reset arena effectively for next run?
    // Arena grows indefinitely if we intern strings every loop?
    // Actually StringInterner doesn't reset.
    // But lexer internals don't allocate much in arena except interner.
    // We should move arena/interner OUTSIDE the loop if we want to measure just
    // lexing speed, but realistic usage includes interning. However, repeated
    // runs usage... If we move interner out, we are testing "warm" interner
    // speed (lookup). If we keep it in, we test allocation speed. Let's keep it
    // out to avoid OOM in long benchmark runs.
  }

  state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(source.size()));
}
BENCHMARK(BM_Lexer);

BENCHMARK_MAIN();
