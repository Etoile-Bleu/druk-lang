#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_array_len(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                    llvm::PointerType* packed_ptr_ty)
{
    auto ops = inst->getOperands();
    if (ops.size() < 1)
        return;

    llvm::Value* val = get_llvm_value(ops[0]);
    if (!val)
        return;

    llvm::Value* res = create_entry_alloca(packed_value_ty);
    ctx_->builder->CreateCall(
        ctx_->module->getOrInsertFunction(
            "druk_jit_len",
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                    {packed_ptr_ty, packed_ptr_ty}, false)),
        {val, res});

    ctx_->ir_values[inst] = res;
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
