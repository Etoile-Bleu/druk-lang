#ifdef DRUK_HAVE_LLVM
// DRUK_BUILD_FIX_v1
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/AtomicOrdering.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/ir/ir_module.h"

namespace druk::codegen
{

LLVMBackend::CompiledFunc LLVMBackend::compileFunction(ir::Function* function)
{
    if (!function)
        return nullptr;
    ir::Module* irModule = function->getParent();
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

    if (irModule)
    {
        prepare_functions_and_wrappers(*irModule, packed_value_ty, packed_ptr_ty);

        for (const auto& [name, irFunc] : irModule->getFunctions())
        {
            compile_single_function(irFunc.get(), packed_value_ty, packed_ptr_ty, i64_ty);
        }
    }

    for (const auto& [irFunc, llvmFunc] : ctx_->ir_functions)
    {
        if (llvm::verifyFunction(*llvmFunc, &llvm::errs()))
            return nullptr;
    }

    optimize_module();
    auto tsm = llvm::orc::ThreadSafeModule(std::move(ctx_->module), std::move(ctx_->context));
    llvm::cantFail(ctx_->jit->addIRModule(std::move(tsm)));

    std::string func_name = function->getName();
    if (func_name.empty() || func_name == "main")
        func_name = "druk_entry";

    if (auto sym_lookup = ctx_->jit->lookup(func_name))
    {
        auto compiled = reinterpret_cast<CompiledFunc>(sym_lookup->getValue());
        ctx_->context.reset(new llvm::LLVMContext());
        ctx_->module.reset(new llvm::Module("druk", *ctx_->context));
        ctx_->builder.reset(new llvm::IRBuilder<>(*ctx_->context));
        ctx_->module->setDataLayout(ctx_->jit->getDataLayout());
        ctx_->module->setTargetTriple(ctx_->jit->getTargetTriple());
        return compiled;
    }
    return nullptr;
}

void LLVMBackend::prepare_functions_and_wrappers(ir::Module&        module,
                                                 llvm::StructType*  packed_value_ty,
                                                 llvm::PointerType* packed_ptr_ty)
{
    for (const auto& [name, irFunc] : module.getFunctions())
    {
        std::string funcName = irFunc->getName();
        if (funcName.empty() || funcName == "main")
            funcName = "druk_entry";

        // 1. Create the implementation function: void (out*, arg1*, arg2*, ...)
        std::vector<llvm::Type*> implParamTypes;
        implParamTypes.push_back(packed_ptr_ty);  // out
        for (size_t i = 0; i < irFunc->getParameterCount(); ++i)
            implParamTypes.push_back(packed_ptr_ty);

        llvm::FunctionType* implFuncType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context), implParamTypes, false);

        std::string     implName = funcName + "_impl";
        llvm::Function* llvmImpl = llvm::Function::Create(
            implFuncType, llvm::Function::ExternalLinkage, implName, ctx_->module.get());

        ctx_->ir_functions[irFunc.get()] = llvmImpl;

        // 2. Create the wrapper function: void (out*)
        // This is the one that JIT runtime will see as the function address.
        llvm::FunctionType* wrapFuncType =
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context), {packed_ptr_ty}, false);

        llvm::Function* llvmWrap = llvm::Function::Create(
            wrapFuncType, llvm::Function::ExternalLinkage, funcName, ctx_->module.get());

        ctx_->ir_wrappers[irFunc.get()] = llvmWrap;
        ctx_->llvm_function             = llvmWrap;  // Set for create_entry_alloca

        llvm::BasicBlock* wrapBB = llvm::BasicBlock::Create(*ctx_->context, "entry", llvmWrap);
        ctx_->builder->SetInsertPoint(wrapBB);

        std::vector<llvm::Value*> callArgs;
        callArgs.push_back(llvmWrap->getArg(0));  // out

        for (size_t i = 0; i < irFunc->getParameterCount(); ++i)
        {
            llvm::Value* argOut = create_entry_alloca(packed_value_ty, "arg_" + std::to_string(i));
            ctx_->builder->CreateCall(
                ctx_->module->getOrInsertFunction(
                    "druk_jit_get_arg",
                    llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                            {llvm::Type::getInt32Ty(*ctx_->context), packed_ptr_ty},
                                            false)),
                {llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx_->context), (uint32_t)i + 1),
                 argOut});
            callArgs.push_back(argOut);
        }

        ctx_->builder->CreateCall(llvmImpl, callArgs);
        ctx_->builder->CreateRetVoid();
    }
}

}  // namespace druk::codegen
#endif
