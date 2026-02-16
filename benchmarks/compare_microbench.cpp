#include "druk/codegen/chunk.hpp"
#include "druk/codegen/generator.hpp"
#include "druk/common/allocator.hpp"
#include "druk/common/error.hpp"
#include "druk/lexer/lexer.hpp"
#include "druk/lexer/unicode.hpp"
#include "druk/parser/core/parser.hpp"
#include "druk/semantic/analyzer.hpp"
#include "druk/vm/vm.hpp"

#ifdef DRUK_HAVE_LLVM
#include "druk/codegen/llvm_jit.hpp"
#endif

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace {

constexpr int64_t kN = 100; // 10 million
constexpr int kRuns = 5;

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

// Baseline C++ - using volatile to prevent over-optimization
int64_t sum_native(int64_t n) {
  volatile int64_t acc = 0;
  for (int64_t i = 0; i < n; ++i) {
    acc += i;
  }
  return acc;
}

double ns_per_op(double seconds, int64_t ops) {
  return (seconds / static_cast<double>(ops)) * 1e9;
}

template <typename Fn>
double measure_best_seconds(Fn &&fn) {
  double best = std::numeric_limits<double>::max();
  for (int i = 0; i < kRuns; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    fn();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    if (elapsed.count() < best) {
      best = elapsed.count();
    }
  }
  return best;
}

} // namespace

int main() {
  DrukProgram prog;
  if (!prog.ok) {
    std::cerr << "Benchmark setup failed: " << prog.error << "\n";
    return 1;
  }

  std::cout << std::fixed << std::setprecision(2);
  std::cout << "N=" << kN << " runs=" << kRuns << "\n";

  // C++ baseline
  auto cpp_seconds = measure_best_seconds([&]() {
    volatile auto result = sum_native(kN);
    (void)result;
  });
  std::cout << "C++:   ns/op=" << ns_per_op(cpp_seconds, kN) << "\n";

#ifdef DRUK_HAVE_LLVM
  // Druk JIT (warmup once to compile)
  druk::JITEngine jit;
  if (!jit.is_available()) {
    std::cout << "JIT:   not available\n";
    return 0;
  }

  auto warmup = jit.execute(prog.function.get());
  (void)warmup;

  auto jit_seconds = measure_best_seconds([&]() {
    auto result = jit.execute(prog.function.get());
    (void)result;
  });
  std::cout << "JIT:   ns/op=" << ns_per_op(jit_seconds, kN) << "\n";
#else
  std::cout << "JIT:   not available\n";
#endif

  return 0;
}
