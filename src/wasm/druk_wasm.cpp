#include "druk/codegen/chunk.hpp"
#include "druk/codegen/generator.hpp"
#include "druk/common/allocator.hpp"
#include "druk/common/error.hpp"
#include "druk/lexer/lexer.hpp"
#include "druk/parser/core/parser.hpp"
#include "druk/semantic/analyzer.hpp"
#include "druk/vm/vm.hpp"

#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <iostream>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

namespace {

std::string run_source(const std::string &source) {
  druk::ArenaAllocator arena;
  druk::ErrorReporter errors;
  druk::StringInterner interner(arena);

  druk::Parser parser(source, arena, interner, errors);
  auto statements = parser.parse();

  if (errors.has_errors()) {
    std::ostringstream err;
    auto *old = std::cerr.rdbuf(err.rdbuf());
    errors.print(source);
    std::cerr.rdbuf(old);
    return err.str();
  }

  druk::SymbolTable symtab;
  druk::SemanticAnalyzer analyzer(symtab, errors, interner, source);
  analyzer.analyze(statements);

  if (errors.has_errors()) {
    std::ostringstream err;
    auto *old = std::cerr.rdbuf(err.rdbuf());
    errors.print(source);
    std::cerr.rdbuf(old);
    return err.str();
  }

  druk::Chunk chunk;
  druk::Generator generator(chunk, interner, errors, source);
  generator.generate(statements);

  if (errors.has_errors()) {
    std::ostringstream err;
    auto *old = std::cerr.rdbuf(err.rdbuf());
    errors.print(source);
    std::cerr.rdbuf(old);
    return err.str();
  }

  druk::VM vm;
  auto function = generator.function();
  function->chunk = std::move(chunk);
  function->name = "script";

  std::ostringstream out;
  auto *old = std::cout.rdbuf(out.rdbuf());
  vm.interpret(function);
  std::cout.rdbuf(old);
  return out.str();
}

} // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE const char *druk_wasm_run(const char *source_utf8) {
  static std::string output;
  output = run_source(source_utf8 ? source_utf8 : "");

  char *buffer = static_cast<char *>(std::malloc(output.size() + 1));
  if (!buffer) {
    return nullptr;
  }
  std::memcpy(buffer, output.c_str(), output.size());
  buffer[output.size()] = '\0';
  return buffer;
}

EMSCRIPTEN_KEEPALIVE void druk_wasm_free(char *ptr) { std::free(ptr); }

}
