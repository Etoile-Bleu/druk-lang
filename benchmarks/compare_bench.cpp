#include "druk/codegen/chunk.hpp"
#include "druk/codegen/generator.hpp"
#include "druk/common/allocator.hpp"
#include "druk/common/error.hpp"
#include "druk/lexer/lexer.hpp"
#include "druk/lexer/unicode.hpp"
#include "druk/parser/core/parser.hpp"
#include "druk/semantic/analyzer.hpp"
#include "druk/vm/vm.hpp"

#include <benchmark/benchmark.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

constexpr int64_t kN = 1000000000;

std::string read_file(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return {};
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::filesystem::path benchmark_dir() {
  return std::filesystem::path(__FILE__).parent_path();
}

struct DrukProgram {
  druk::ArenaAllocator arena;
  druk::StringInterner interner;
  std::shared_ptr<druk::ObjFunction> function;
  bool ok = false;
  std::string error;

  DrukProgram() : arena(), interner(arena) {
    const auto source_path = benchmark_dir() / "compare.druk";
    const auto source = read_file(source_path);
    if (source.empty()) {
      error = "Failed to load Druk source: " + source_path.string();
      return;
    }

    druk::ErrorReporter errors;
    druk::Parser parser(source, arena, interner, errors);
    auto statements = parser.parse();
    if (errors.has_errors()) {
      error = "Parse errors in compare.druk";
      return;
    }

    druk::SymbolTable symtab;
    druk::SemanticAnalyzer analyzer(symtab, errors, interner, source);
    analyzer.analyze(statements);
    if (errors.has_errors()) {
      error = "Semantic errors in compare.druk";
      return;
    }

    druk::Chunk chunk;
    druk::Generator generator(chunk, interner, errors, source);
    generator.generate(statements);
    if (errors.has_errors()) {
      error = "Codegen errors in compare.druk";
      return;
    }

    auto func = std::make_shared<druk::ObjFunction>();
    func->chunk = std::move(chunk);
    func->name = "bench";
    function = std::move(func);
    ok = true;
  }
};

DrukProgram &program() {
  static DrukProgram prog;
  return prog;
}

int64_t sum_native(int64_t n) {
  int64_t acc = 0;
  for (int64_t i = 0; i < n; ++i) {
    acc += i;
  }
  return acc;
}

} // namespace

static void BM_CPP_Sum(benchmark::State &state) {
  for (auto _ : state) {
    auto result = sum_native(kN);
    benchmark::DoNotOptimize(result);
  }
  state.SetItemsProcessed(state.iterations() * kN);
}

static void BM_Druk_Sum(benchmark::State &state) {
  auto &prog = program();
  if (!prog.ok) {
    state.SkipWithError(prog.error.c_str());
    return;
  }

  for (auto _ : state) {
    druk::VM vm;
    vm.interpret(prog.function);
    benchmark::DoNotOptimize(vm);
  }
  state.SetItemsProcessed(state.iterations() * kN);
}

BENCHMARK(BM_CPP_Sum);
BENCHMARK(BM_Druk_Sum);
