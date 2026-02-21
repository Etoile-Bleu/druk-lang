#ifdef DRUK_HAVE_LLVM

#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <stdexcept>

#include "druk/codegen/llvm/llvm_backend.h"

extern "C"
{
    void LLVMInitializeX86TargetInfo();
    void LLVMInitializeX86Target();
    void LLVMInitializeX86TargetMC();
    void LLVMInitializeX86AsmPrinter();
}

namespace druk::codegen
{

LLVMBackend::CompilationContext::CompilationContext()
    : context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>("druk", *context)),
      builder(std::make_unique<llvm::IRBuilder<>>(*context))
{
}

LLVMBackend::LLVMBackend(bool debug) : debug_(debug), ctx_(std::make_unique<CompilationContext>())
{
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmPrinter();

    std::string         triple_str = llvm::sys::getDefaultTargetTriple();
    llvm::Triple        triple{llvm::Twine{triple_str}};
    std::string         error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target)
        throw std::runtime_error("Failed to lookup target: " + error);

    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
    auto jit_expected = llvm::orc::LLJITBuilder().setLinkProcessSymbolsByDefault(true).create();
    if (!jit_expected)
        throw std::runtime_error("Failed to create LLJIT: " +
                                 llvm::toString(jit_expected.takeError()));
    ctx_->jit = std::move(*jit_expected);

    ctx_->module = std::make_unique<llvm::Module>("druk", *ctx_->context);
    ctx_->module->setDataLayout(ctx_->jit->getDataLayout());
    ctx_->module->setTargetTriple(ctx_->jit->getTargetTriple());

    register_runtime_symbols();
    register_extended_symbols();
}

LLVMBackend::~LLVMBackend() = default;

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
