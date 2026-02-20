#pragma once
#include <string>

#include "druk/ir/ir_module.h"


namespace druk::codegen
{

class LLVMCodeGen
{
   public:
    explicit LLVMCodeGen(bool debug = false);
    ~LLVMCodeGen();
    bool emitMachineCode(ir::Module& module, const std::string& outputPath);
    void runJIT(ir::Module& module);

   private:
    std::string find_linker();
    bool        link_executable(const std::string& obj_path, const std::string& exe_path);
    bool        debug_;
};

}  // namespace druk::codegen
