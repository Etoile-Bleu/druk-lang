#include "druk/lexer/lexer.hpp"
#include "druk/common/allocator.hpp"
#include "druk/common/error.hpp"

#include <benchmark/benchmark.h>
#include <string>

namespace {

// Simple Druk program for benchmarking
const char *kSimpleProgram = R"(
ལས་ཀ་ fibonacci(གྲངས་ཀ་ n) {
    གལ་ཏེ་ (n < ༢) {
        ལོག་ n;
    }
    ལོག་ fibonacci(n - ༡) + fibonacci(n - ༢);
}

གྲངས་ཀ་ result = fibonacci(༡༠);
འབྲི་ result;
)";

// ASCII-heavy program
const char *kAsciiProgram = R"(
function fibonacci(number n) {
    if (n < 2) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

number result = fibonacci(10);
print result;
)";

} // namespace

static void BM_Lexer_Tibetan(benchmark::State &state) {
  for (auto _ : state) {
    druk::ArenaAllocator arena;
    druk::StringInterner interner(arena);
    druk::ErrorReporter errors;
    druk::Lexer lexer(kSimpleProgram, arena, interner, errors);

    size_t token_count = 0;
    while (true) {
      auto token = lexer.next();
      token_count++;
      if (token.kind == druk::TokenKind::EndOfFile) {
        break;
      }
    }
    benchmark::DoNotOptimize(token_count);
  }
  state.SetItemsProcessed(state.iterations());
}

static void BM_Lexer_ASCII(benchmark::State &state) {
  for (auto _ : state) {
    druk::ArenaAllocator arena;
    druk::StringInterner interner(arena);
    druk::ErrorReporter errors;
    druk::Lexer lexer(kAsciiProgram, arena, interner, errors);

    size_t token_count = 0;
    while (true) {
      auto token = lexer.next();
      token_count++;
      if (token.kind == druk::TokenKind::EndOfFile) {
        break;
      }
    }
    benchmark::DoNotOptimize(token_count);
  }
  state.SetItemsProcessed(state.iterations());
}

// Large file benchmark
static void BM_Lexer_Large(benchmark::State &state) {
  // Generate a large program
  std::string large_program;
  for (int i = 0; i < 1000; i++) {
    large_program += kSimpleProgram;
    large_program += "\n";
  }

  for (auto _ : state) {
    druk::ArenaAllocator arena;
    druk::StringInterner interner(arena);
    druk::ErrorReporter errors;
    druk::Lexer lexer(large_program, arena, interner, errors);

    size_t token_count = 0;
    while (true) {
      auto token = lexer.next();
      token_count++;
      if (token.kind == druk::TokenKind::EndOfFile) {
        break;
      }
    }
    benchmark::DoNotOptimize(token_count);
  }
  state.SetBytesProcessed(state.iterations() * large_program.size());
}

BENCHMARK(BM_Lexer_Tibetan);
BENCHMARK(BM_Lexer_ASCII);
BENCHMARK(BM_Lexer_Large);

BENCHMARK_MAIN();
