#pragma once

#ifdef DRUK_HAVE_LLVM

#include <memory>
#include <optional>
#include <string>

#include "druk/codegen/core/obj.h"
#include "druk/ir/ir_function.h"

namespace druk::codegen
{

class LLVMBackend;

class JITEngine
{
   public:
    explicit JITEngine(bool debug = false);
    ~JITEngine();

    JITEngine(const JITEngine&)            = delete;
    JITEngine& operator=(const JITEngine&) = delete;

    std::optional<int64_t> execute(ir::Function* function);

    bool isAvailable() const;

    struct Stats
    {
        size_t functionsCompiled  = 0;
        size_t totalInstructions  = 0;
        double totalCompileTimeMs = 0.0;
    };

    Stats getStats() const
    {
        return stats_;
    }
    void resetStats()
    {
        stats_ = Stats{};
    }

   private:
    bool                         debug_ = false;
    std::unique_ptr<LLVMBackend> backend_;
    Stats                        stats_;
};

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
