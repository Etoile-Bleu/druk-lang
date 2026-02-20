#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_null_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                   llvm::PointerType* packed_ptr_ty)
{
    if (inst->getOpcode() == ir::Opcode::Unwrap)
    {
        llvm::Value* operand = get_llvm_value(inst->getOperands()[0]);
        
        // operand is PackedValue*, so GEP and Load the type byte
        llvm::Value* type_ptr = ctx_->builder->CreateStructGEP(packed_value_ty, operand, 0, "type_ptr");
        llvm::Value* type_val = ctx_->builder->CreateLoad(ctx_->builder->getInt8Ty(), type_ptr, "unwrap_type");
        
        llvm::Value* is_nil = ctx_->builder->CreateICmpEQ(
            type_val, llvm::ConstantInt::get(ctx_->builder->getInt8Ty(), 0), "is_nil_cond");

        llvm::BasicBlock* panic_bb = llvm::BasicBlock::Create(*ctx_->context, "unwrap_panic", ctx_->llvm_function);
        llvm::BasicBlock* cont_bb  = llvm::BasicBlock::Create(*ctx_->context, "unwrap_cont", ctx_->llvm_function);

        ctx_->builder->CreateCondBr(is_nil, panic_bb, cont_bb);

        // Panic Block
        ctx_->builder->SetInsertPoint(panic_bb);
        llvm::FunctionCallee panic_func = ctx_->module->getOrInsertFunction(
            "druk_jit_panic_unwrap", llvm::FunctionType::get(ctx_->builder->getVoidTy(), false));
        ctx_->builder->CreateCall(panic_func);
        ctx_->builder->CreateUnreachable(); // It will abort execution

        // Continue Block
        ctx_->builder->SetInsertPoint(cont_bb);
        
        // Unwrap returns the exact same runtime value structure since optional and base types share PackedValue
        ctx_->ir_values[inst] = operand;
    }
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
