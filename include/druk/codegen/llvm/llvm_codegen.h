#pragma once

#include <string>

#include "druk/ir/ir_module.h"


namespace druk::codegen
{

/**
 * @brief LLVM backend that translates Druk IR to LLVM IR and generates machine code.
 */
class LLVMCodeGen
{
   public:
    explicit LLVMCodeGen(bool debug = false);
    ~LLVMCodeGen();

    bool emitMachineCode(ir::Module& module, const std::string& outputPath);
    void runJIT(ir::Module& module);

   private:
    bool debug_;
    // LLVM specific members will be added in implementation
};

}  // namespace druk::codegen
