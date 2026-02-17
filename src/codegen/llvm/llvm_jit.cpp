#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_jit.h"

#include <chrono>
#include <iostream>

#include "druk/codegen/jit/jit_runtime.h"
#include "druk/codegen/llvm/llvm_backend.h"

namespace druk::codegen
{

namespace
{
LLVMBackend* g_backend = nullptr;

// Legacy thunk removed as we no longer support ObjFunction compilation via LLVM
}  // namespace

JITEngine::JITEngine(bool debug) : debug_(debug)
{
    try
    {
        backend_  = std::make_unique<LLVMBackend>(debug_);
        g_backend = backend_.get();
        // druk_jit_set_compile_handler(&compile_thunk); // Removed
    }
    catch (const std::exception&)
    {
        backend_ = nullptr;
    }
}

JITEngine::~JITEngine()
{
    if (g_backend == backend_.get())
    {
        g_backend = nullptr;
        druk_jit_set_compile_handler(nullptr);
    }
}

std::optional<int64_t> JITEngine::execute(ir::Function* function)
{
    if (!backend_ || !function)
    {
        return std::nullopt;
    }

    auto   start       = std::chrono::high_resolution_clock::now();
    auto   compiled    = backend_->compileFunction(function);
    auto   end         = std::chrono::high_resolution_clock::now();
    double compileTime = std::chrono::duration<double, std::milli>(end - start).count();

    if (!compiled)
        return std::nullopt;

    stats_.functionsCompiled++;
    // stats_.totalInstructions += function->chunk.code().size(); // IR instruction count?
    stats_.totalCompileTimeMs += compileTime;

    PackedValue result{};
    compiled(&result);
    // For now, return 0 if void, or int if possible. The signature is void(PackedValue*).
    // result should be populated by the function.
    return druk_jit_value_as_int(&result);
}

bool JITEngine::isAvailable() const
{
    return backend_ != nullptr;
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
