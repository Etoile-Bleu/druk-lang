#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_string_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                     llvm::PointerType* packed_ptr_ty)
{
    if (inst->getOpcode() == ir::Opcode::ToString)
    {
        ir::Value*   argOperand = inst->getOperands()[0];
        llvm::Value* argValue   = get_llvm_value(argOperand);

        llvm::Value* outValue = create_entry_alloca(packed_value_ty, inst->getName() + "_out");

        llvm::FunctionType* to_string_ty = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*ctx_->context), {packed_ptr_ty, packed_ptr_ty}, false);

        ctx_->builder->CreateCall(ctx_->module->getOrInsertFunction("druk_jit_to_string", to_string_ty),
                                  {argValue, outValue});

        ctx_->ir_values[inst] = outValue;
    }
    else if (inst->getOpcode() == ir::Opcode::StringConcat)
    {
        ir::Value*   leftOperand  = inst->getOperands()[0];
        ir::Value*   rightOperand = inst->getOperands()[1];
        llvm::Value* leftValue    = get_llvm_value(leftOperand);
        llvm::Value* rightValue   = get_llvm_value(rightOperand);

        llvm::Value* outValue = create_entry_alloca(packed_value_ty, inst->getName() + "_out");

        llvm::FunctionType* concat_ty = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*ctx_->context),
            {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false);

        ctx_->builder->CreateCall(
            ctx_->module->getOrInsertFunction("druk_jit_string_concat", concat_ty),
            {leftValue, rightValue, outValue});

        ctx_->ir_values[inst] = outValue;
    }
}

}  // namespace druk::codegen

#endif
