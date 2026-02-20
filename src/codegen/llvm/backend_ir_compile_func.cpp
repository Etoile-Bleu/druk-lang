#ifdef DRUK_HAVE_LLVM
#include <llvm/IR/Verifier.h>

#include "druk/codegen/llvm/llvm_backend.h"


namespace druk::codegen
{

void LLVMBackend::compile_single_function(ir::Function* function, llvm::StructType* packed_value_ty,
                                          llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
{
    ctx_->ir_values.clear();
    ctx_->ir_blocks.clear();
    ctx_->current_ir_function = function;
    llvm::Function* llvmFunc  = ctx_->ir_functions[function];
    ctx_->llvm_function       = llvmFunc;
    ctx_->ret_out             = llvmFunc->getArg(0);

    for (size_t i = 0; i < function->getParameterCount(); ++i)
    {
        ir::Parameter*  irParam   = function->getParameter(i);
        llvm::Argument* llvmParam = llvmFunc->getArg(i + 1);
        ctx_->ir_values[irParam]  = llvmParam;
    }

    for (const auto& bb : function->getBasicBlocks())
    {
        llvm::BasicBlock* llvmBB =
            llvm::BasicBlock::Create(*ctx_->context, bb->getName(), llvmFunc);
        ctx_->ir_blocks[bb.get()] = llvmBB;
    }

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
#endif
