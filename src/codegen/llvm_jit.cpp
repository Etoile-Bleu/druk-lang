#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm_jit.hpp"
#include "druk/codegen/llvm_backend.hpp"
#include "druk/codegen/jit_runtime.hpp"
#include <chrono>
#include <iostream>

namespace druk {

namespace {
LLVMBackend *g_backend = nullptr;

DrukJitFunc compile_thunk(ObjFunction *function) {
  if (!g_backend || !function) {
    return nullptr;
  }
  return g_backend->compile_function(function);
}
} // namespace

JITEngine::JITEngine(bool debug) : debug_(debug) {
  try {
    if (debug_) {
      std::cerr << "[JITEngine] Creating backend...\n";
    }
    backend_ = std::make_unique<LLVMBackend>(debug_);
    if (debug_) {
      std::cerr << "[JITEngine] Backend created successfully\n";
    }
    g_backend = backend_.get();
    druk_jit_set_compile_handler(&compile_thunk);
  } catch (const std::exception &e) {
    // JIT not available
    if (debug_) {
      std::cerr << "[JITEngine] Failed to create backend: " << e.what() << "\n";
    }
    backend_ = nullptr;
  }
}

JITEngine::~JITEngine() {
  if (g_backend == backend_.get()) {
    g_backend = nullptr;
    druk_jit_set_compile_handler(nullptr);
  }
}

std::optional<int64_t> JITEngine::execute(ObjFunction *function) {
  if (!backend_ || !function) {
    return std::nullopt;
  }

  auto start = std::chrono::high_resolution_clock::now();

  // Compile the function
  auto compiled = backend_->compile_function(function);
  
  auto end = std::chrono::high_resolution_clock::now();
  double compile_time =
      std::chrono::duration<double, std::milli>(end - start).count();

  if (!compiled) {
    return std::nullopt;
  }

  // Update statistics
  stats_.functions_compiled++;
  stats_.total_instructions += function->chunk.code().size();
  stats_.total_compile_time_ms += compile_time;

  // Execute the compiled function
  PackedValue result{};
  compiled(&result);
  return druk_jit_value_as_int(&result);
}

bool JITEngine::can_compile(ObjFunction *function) const {
  return backend_ && function;
}

} // namespace druk

#endif // DRUK_HAVE_LLVM
