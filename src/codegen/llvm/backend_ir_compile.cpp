#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include "druk/ir/ir_module.h"

namespace druk::codegen
{

LLVMBackend::CompiledFunc LLVMBackend::compileFunction(ir::Function* function)
{
    if (!function)
        return nullptr;

    // Get the parent module to compile all functions
    ir::Module* irModule = function->getParent();
    
    ctx_->ir_values.clear();
    ctx_->ir_blocks.clear();
    ctx_->ir_functions.clear();

    llvm::Type*       i8_ty      = llvm::Type::getInt8Ty(*ctx_->context);
    llvm::Type*       i64_ty     = llvm::Type::getInt64Ty(*ctx_->context);
    llvm::ArrayType*  padding_ty = llvm::ArrayType::get(i8_ty, 7);
    llvm::StructType* packed_value_ty =
        llvm::StructType::get(*ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);
    llvm::PointerType* packed_ptr_ty = llvm::PointerType::getUnqual(*ctx_->context);

    llvm::FunctionType* func_type =
        llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context), {packed_ptr_ty}, false);

    // Step 1: Create LLVM function declarations for all IR functions
    if (irModule)
    {
        for (const auto& [name, irFunc] : irModule->getFunctions())
        {
            std::string funcName = irFunc->getName();
            if (funcName.empty())
                funcName = "main";
            
            // Create function type with parameters
            std::vector<llvm::Type*> paramTypes;
            paramTypes.push_back(packed_ptr_ty);  // Return value (out parameter)
            
            // Add function parameters
            for (size_t i = 0; i < irFunc->getParameterCount(); ++i)
            {
                paramTypes.push_back(packed_ptr_ty);  // Each parameter is PackedValue*
            }
            
            llvm::FunctionType* funcType = llvm::FunctionType::get(
                llvm::Type::getVoidTy(*ctx_->context), paramTypes, false);
            
            llvm::Function* llvmFunc = llvm::Function::Create(
                funcType, llvm::Function::ExternalLinkage, funcName, ctx_->module.get());
            
            ctx_->ir_functions[irFunc.get()] = llvmFunc;
        }
    }

    // Step 2: Compile all IR functions
    if (irModule)
    {
        for (const auto& [name, irFunc] : irModule->getFunctions())
        {
            compile_single_function(irFunc.get(), packed_value_ty, packed_ptr_ty, i64_ty);
        }
    }

    // Verify all functions
    for (const auto& [irFunc, llvmFunc] : ctx_->ir_functions)
    {
        if (llvm::verifyFunction(*llvmFunc, &llvm::errs()))
            return nullptr;
    }

    optimize_module();

    auto tsm = llvm::orc::ThreadSafeModule(std::move(ctx_->module), std::move(ctx_->context));
    llvm::cantFail(ctx_->jit->addIRModule(std::move(tsm)));

    std::string func_name = function->getName();
    if (func_name.empty())
        func_name = "main";

    if (auto sym_lookup = ctx_->jit->lookup(func_name))
    {
        auto compiled = reinterpret_cast<CompiledFunc>(sym_lookup->getValue());

        ctx_->context.reset(new llvm::LLVMContext());
        ctx_->module.reset(new llvm::Module("druk", *ctx_->context));
        ctx_->builder.reset(new llvm::IRBuilder<>(*ctx_->context));

        ctx_->module->setDataLayout(ctx_->jit->getDataLayout());
        ctx_->module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));

        return compiled;
    }
    return nullptr;
}

void LLVMBackend::compile_single_function(ir::Function* function,
                                          llvm::StructType* packed_value_ty,
                                          llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
{
    // Clear IR value and block mappings for this function
    ctx_->ir_values.clear();
    ctx_->ir_blocks.clear();
    
    ctx_->current_ir_function = function;
    llvm::Function* llvmFunc  = ctx_->ir_functions[function];
    ctx_->llvm_function       = llvmFunc;
    ctx_->ret_out             = llvmFunc->getArg(0);

    // Map IR parameters to LLVM function arguments
    for (size_t i = 0; i < function->getParameterCount(); ++i)
    {
        ir::Parameter* irParam    = function->getParameter(i);
        llvm::Argument* llvmParam = llvmFunc->getArg(i + 1);  // +1 because arg 0 is return value
        ctx_->ir_values[irParam]  = llvmParam;
    }

    // Create basic blocks
    for (const auto& bb : function->getBasicBlocks())
    {
        llvm::BasicBlock* llvmBB =
            llvm::BasicBlock::Create(*ctx_->context, bb->getName(), llvmFunc);
        ctx_->ir_blocks[bb.get()] = llvmBB;
    }

    // Compile instructions in each basic block
    for (const auto& bb : function->getBasicBlocks())
    {
        llvm::BasicBlock* llvmBB = ctx_->ir_blocks[bb.get()];
        ctx_->builder->SetInsertPoint(llvmBB);

        for (const auto& inst : *bb)
        {
            compile_instruction(inst.get(), packed_value_ty, packed_ptr_ty, i64_ty);
        }
    }
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
