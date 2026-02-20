#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_array_index(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                      llvm::PointerType* packed_ptr_ty)
{
    auto ops = inst->getOperands();
    if (inst->getOpcode() == ir::Opcode::IndexGet)
    {
        if (ops.size() < 2)
            return;

        llvm::Value* arr_val = get_llvm_value(ops[0]);
        llvm::Value* idx_val = get_llvm_value(ops[1]);
        if (!arr_val || !idx_val)
            return;

        llvm::Value* res = create_entry_alloca(packed_value_ty);
        ctx_->builder->CreateCall(
            ctx_->module->getOrInsertFunction(
                "druk_jit_index",
                llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                        {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false)),
            {arr_val, idx_val, res});

        ctx_->ir_values[inst] = res;
        return;
    }

    if (ops.size() < 3)
        return;

    llvm::Value* arr_val = get_llvm_value(ops[0]);
    llvm::Value* idx_val = get_llvm_value(ops[1]);
    llvm::Value* val = get_llvm_value(ops[2]);
    if (!arr_val || !idx_val || !val)
        return;

    ctx_->builder->CreateCall(
        ctx_->module->getOrInsertFunction(
            "druk_jit_index_set",
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                    {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false)),
        {arr_val, idx_val, val});
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
