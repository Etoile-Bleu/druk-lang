#ifdef DRUK_HAVE_LLVM

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/ir/ir_module.h"

namespace druk::codegen
{

bool LLVMBackend::emitObjectFile(ir::Module& module, const std::string& obj_path)
{
    ctx_->ir_values.clear();
    ctx_->ir_blocks.clear();
    ctx_->ir_functions.clear();
    ctx_->ir_wrappers.clear();

    llvm::Type*       i8_ty      = llvm::Type::getInt8Ty(*ctx_->context);
    llvm::Type*       i64_ty     = llvm::Type::getInt64Ty(*ctx_->context);
    llvm::ArrayType*  padding_ty = llvm::ArrayType::get(i8_ty, 7);
    llvm::StructType* packed_value_ty =
        llvm::StructType::get(*ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
    llvm::PointerType* packed_ptr_ty = llvm::PointerType::getUnqual(*ctx_->context);

    prepare_functions_and_wrappers(module, packed_value_ty, packed_ptr_ty);

    for (const auto& [name, irFunc] : module.getFunctions())
    {
        compile_single_function(irFunc.get(), packed_value_ty, packed_ptr_ty, i64_ty);
    }

    // Create a C-compatible main() wrapper
    // int main(int argc, char** argv)
    {
        llvm::FunctionType* mainType =
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx_->context),
                                    {
                                        llvm::Type::getInt32Ty(*ctx_->context),       // argc
                                        llvm::PointerType::getUnqual(*ctx_->context)  // argv
                                    },
                                    false);

        llvm::Function* mainFunc = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage,
                                                          "main", ctx_->module.get());

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(*ctx_->context, "entry", mainFunc);
        llvm::IRBuilder<> builder(entry);

        // Initialize Runtime (Console UTF-8, etc.)
        llvm::FunctionType* initType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context), false);
        llvm::Function* initFunc = llvm::Function::Create(initType, llvm::Function::ExternalLinkage,
                                                          "druk_runtime_init", ctx_->module.get());
        builder.CreateCall(initFunc);

        // Allocate return value for druk_entry
        llvm::Value* retVal = builder.CreateAlloca(packed_value_ty, nullptr, "retval");

        // Call druk_entry
        // Find the entry function. It corresponds to the one we named "druk_entry" above.
        llvm::Function* drukEntry = ctx_->module->getFunction("druk_entry");
        if (drukEntry)
        {
            // druk_entry(PackedValue* out, ...)
            // Assuming script entry takes no extra args, just the return pointer.
            // If it takes args, we need to handle them (e.g. CLI args passed to script).
            // For now, we pass the retVal pointer.
            std::vector<llvm::Value*> args;
            args.push_back(retVal);

            // Check if druk_entry expects more args (it shouldn't for top-level script, but safety
            // check)
            if (drukEntry->arg_size() > args.size())
            {
                // Pad with nulls or handle appropriately?
                // Druk script top-level usually has 0 params (plus implicit ret pointer = 1 arg).
                for (size_t i = args.size(); i < drukEntry->arg_size(); ++i)
                {
                    args.push_back(llvm::Constant::getNullValue(packed_ptr_ty));
                }
            }

            builder.CreateCall(drukEntry, args);
        }

        // Return 0
        builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx_->context), 0));
    }

    for (const auto& [irFunc, llvmFunc] : ctx_->ir_functions)
    {
        if (llvm::verifyFunction(*llvmFunc, &llvm::errs()))
        {
            llvm::errs() << "AOT: LLVM verify failed for function: " << llvmFunc->getName() << "\n";
            return false;
        }
    }

    std::string         error;
    llvm::Triple        triple(llvm::sys::getDefaultTargetTriple());
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple.str(), error);
    if (!target)
    {
        llvm::errs() << "AOT: Failed to lookup target: " << error << "\n";
        return false;
    }

    llvm::TargetOptions opts;
    auto                tm = std::unique_ptr<llvm::TargetMachine>(target->createTargetMachine(
        triple, "generic", "", opts, std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_),
        std::optional<llvm::CodeModel::Model>(llvm::CodeModel::Small),
        llvm::CodeGenOptLevel::Default));
    if (!tm)
    {
        llvm::errs() << "AOT: Failed to create TargetMachine.\n";
        return false;
    }

    ctx_->module->setDataLayout(tm->createDataLayout());
    ctx_->module->setTargetTriple(triple);

    if (llvm::verifyModule(*ctx_->module, &llvm::errs()))
    {
        llvm::errs() << "AOT: Module verification failed before optimization.\n";
        return false;
    }

    try
    {
        optimize_module();
    }
    catch (...)
    {
        llvm::errs() << "AOT: Optimization (O3) crashed or failed. Retrying with O0...\n";
    }

    if (llvm::verifyModule(*ctx_->module, &llvm::errs()))
    {
        llvm::errs() << "AOT: Module verification failed after optimization.\n";
        return false;
    }

    std::error_code      ec;
    llvm::raw_fd_ostream dest(obj_path, ec, llvm::sys::fs::OF_None);
    if (ec)
    {
        llvm::errs() << "AOT: Failed to open object file: " << ec.message() << "\n";
        return false;
    }

    llvm::legacy::PassManager pass;
    if (tm->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
    {
        llvm::errs() << "AOT: TargetMachine cannot emit object file.\n";
        return false;
    }

    pass.run(*ctx_->module);
    dest.flush();

    // Important: Module must be destroyed BEFORE Context because Module refers to Context.
    ctx_->builder.reset();
    ctx_->module.reset();
    ctx_->context.reset(new llvm::LLVMContext());

    ctx_->module.reset(new llvm::Module("druk", *ctx_->context));
    ctx_->builder.reset(new llvm::IRBuilder<>(*ctx_->context));

    return true;
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
