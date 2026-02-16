#pragma once

#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/obj.hpp"
#include <optional>
#include <memory>
#include <string>

namespace druk {

class LLVMBackend;

/**
 * JIT execution engine for Druk.
 * Provides a high-level interface to compile and execute Druk bytecode
 * using LLVM's JIT compiler.
 */
class JITEngine {
public:
  explicit JITEngine(bool debug = false);
  ~JITEngine();

  // Disable copy and move
  JITEngine(const JITEngine &) = delete;
  JITEngine &operator=(const JITEngine &) = delete;

  /**
   * Compile and execute a function with JIT.
   * Returns the result of execution.
   */
  std::optional<int64_t> execute(ObjFunction *function);

  /**
   * Check if this function can be compiled by the JIT.
   */
  bool can_compile(ObjFunction *function) const;

  /**
   * Check if JIT is available and initialized.
   */
  bool is_available() const { return backend_ != nullptr; }

  /**
   * Get compilation statistics.
   */
  struct Stats {
    size_t functions_compiled = 0;
    size_t total_instructions = 0;
    double total_compile_time_ms = 0.0;
  };
  
  Stats get_stats() const { return stats_; }
  void reset_stats() { stats_ = Stats{}; }

private:
  bool debug_ = false;
  std::unique_ptr<LLVMBackend> backend_;
  Stats stats_;
};

} // namespace druk

#endif // DRUK_HAVE_LLVM
