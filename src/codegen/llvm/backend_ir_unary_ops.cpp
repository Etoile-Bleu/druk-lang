#ifdef DRUK_HAVE_LLVM
#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{
void LLVMBackend::compile_unary_op(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                   llvm::PointerType* packed_ptr_ty)
{
    auto ops = inst->getOperands();
    if (ops.empty()) return;
    
    llvm::Value* val = get_llvm_value(ops[0]);
    if (!val) return;
    
    llvm::Value* res = create_entry_alloca(packed_value_ty);
    const char* fn = "";
    
    switch (inst->getOpcode())
    {
        case ir::Opcode::Neg:
            fn = "druk_jit_negate";
            break;
        case ir::Opcode::Not:
            fn = "druk_jit_not";
            break;
        default:
            return;
    }
    
    ctx_->builder->CreateCall(
        ctx_->module->getOrInsertFunction(
            fn, llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                        {packed_ptr_ty, packed_ptr_ty}, false)),
        {val, res});
    
    ctx_->ir_values[inst] = res;
}
}
#endif
