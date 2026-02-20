#ifdef DRUK_HAVE_LLVM

#include <llvm/IR/BasicBlock.h>
#include <llvm/Passes/PassBuilder.h>

#include <iostream>

#include "druk/codegen/llvm/llvm_backend.h"

namespace druk::codegen
{

void LLVMBackend::optimize_module()
{
    llvm::LoopAnalysisManager     lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager    cgam;
    llvm::ModuleAnalysisManager   mam;
    llvm::PassBuilder             pb;
    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);
    pb.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3).run(*ctx_->module, mam);
}

llvm::Value* LLVMBackend::create_entry_alloca(llvm::Type* type, const std::string& name)
{
    llvm::BasicBlock& entry = ctx_->llvm_function->getEntryBlock();
    llvm::IRBuilder<> tmp_builder(&entry, entry.begin());
    return tmp_builder.CreateAlloca(type, nullptr, name);
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
