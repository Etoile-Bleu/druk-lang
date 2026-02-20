#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <llvm/IR/Constants.h>

#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_array_build(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                      llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
{
    auto ops = inst->getOperands();
    size_t count = ops.size();
    size_t alloc_count = count == 0 ? 1 : count;

    llvm::ArrayType* array_ty = llvm::ArrayType::get(packed_value_ty, alloc_count);
    llvm::Value* array_alloc = create_entry_alloca(array_ty, "array_elems");

    llvm::Value* zero = llvm::ConstantInt::get(i64_ty, 0);
    for (size_t i = 0; i < count; ++i)
    {
        llvm::Value* elem_val = get_llvm_value(ops[i]);
        if (!elem_val)
            return;

        llvm::Value* index = llvm::ConstantInt::get(i64_ty, i);
        llvm::Value* elem_ptr =
            ctx_->builder->CreateInBoundsGEP(array_ty, array_alloc, {zero, index});
        ctx_->builder->CreateMemCpy(elem_ptr, llvm::MaybeAlign(8), elem_val,
                                   llvm::MaybeAlign(8),
                                   llvm::ConstantInt::get(i64_ty, 24));
    }

    llvm::Value* first_ptr =
        ctx_->builder->CreateInBoundsGEP(array_ty, array_alloc, {zero, zero});
    llvm::Value* res = create_entry_alloca(packed_value_ty);
    llvm::Value* count_val =
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*ctx_->context),
                               static_cast<int32_t>(count));

    ctx_->builder->CreateCall(
        ctx_->module->getOrInsertFunction(
            "druk_jit_build_array",
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                    {packed_ptr_ty, llvm::Type::getInt32Ty(*ctx_->context),
                                     packed_ptr_ty},
                                    false)),
        {first_ptr, count_val, res});

    ctx_->ir_values[inst] = res;
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
