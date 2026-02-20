#include <filesystem>
#include <iostream>

#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/codegen/llvm/llvm_codegen.h"


namespace druk::codegen
{

LLVMCodeGen::LLVMCodeGen(bool debug) : debug_(debug) {}
LLVMCodeGen::~LLVMCodeGen() = default;

void LLVMCodeGen::runJIT(ir::Module& module)
{
    std::cout << "Running JIT: " << module.getName() << "\n";
}

bool LLVMCodeGen::emitMachineCode(ir::Module& module, const std::string& outputPath)
{
    std::filesystem::path exe_path(outputPath);
#ifdef _WIN32
    if (exe_path.extension() != ".exe")
        exe_path.replace_extension(".exe");
    std::filesystem::path obj_path = exe_path;
    obj_path.replace_extension(".obj");
#else
    std::filesystem::path obj_path = exe_path;
    obj_path.replace_extension(".o");
#endif
    LLVMBackend backend(debug_);
    if (!backend.emitObjectFile(module, obj_path.string()))
        return false;
    if (!link_executable(obj_path.string(), exe_path.string()))
        return false;
    return std::filesystem::exists(exe_path);
}

}  // namespace druk::codegen
