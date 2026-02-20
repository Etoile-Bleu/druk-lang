#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_print_op(ir::Instruction* inst, llvm::PointerType* packed_ptr_ty)
{
    auto ops = inst->getOperands();
    if (ops.empty())
        return;

    llvm::Value* val = get_llvm_value(ops[0]);
    if (!val)
        return;

    ctx_->builder->CreateCall(
        ctx_->module->getOrInsertFunction(
            "druk_jit_print",
            llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context), {packed_ptr_ty}, false)),
        {val});
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
