#ifdef DRUK_HAVE_LLVM

#include <llvm/IR/Constants.h>

#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_dynamic_call_op(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                          llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
{
    auto* dcall = dynamic_cast<ir::DynamicCallInst*>(inst);
    if (!dcall)
        return;

    llvm::Value* calleeVal = get_llvm_value(dcall->getCallee());

    // Create an array of PackedValue for arguments on the stack
    uint32_t     argCount  = dcall->getOperandCount() - 1;
    llvm::Value* argsArray = nullptr;
    if (argCount > 0)
    {
        llvm::ArrayType* arrayTy = llvm::ArrayType::get(packed_value_ty, argCount);
        argsArray                = create_entry_alloca(arrayTy, "dcall_args");

        for (uint32_t i = 0; i < argCount; ++i)
        {
            llvm::Value* argVal  = get_llvm_value(dcall->getOperand(i + 1));
            llvm::Value* destPtr = ctx_->builder->CreateStructGEP(arrayTy, argsArray, i);
            ctx_->builder->CreateMemCpy(destPtr, llvm::MaybeAlign(8), argVal, llvm::MaybeAlign(8),
                                        llvm::ConstantInt::get(i64_ty, 24));
        }
    }
    else
    {
        argsArray = llvm::ConstantPointerNull::get(packed_ptr_ty);
    }

    llvm::Value* outVal = create_entry_alloca(packed_value_ty, "dcall_out");

    ctx_->builder->CreateCall(
        ctx_->module->getOrInsertFunction(
            "druk_jit_call",
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                    {packed_ptr_ty, packed_ptr_ty,
                                     llvm::Type::getInt32Ty(*ctx_->context), packed_ptr_ty},
                                    false)),
        {calleeVal, ctx_->builder->CreateBitCast(argsArray, packed_ptr_ty),
         llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx_->context), argCount), outVal});

    ctx_->ir_values[inst] = outVal;
}

}  // namespace druk::codegen
#endif
