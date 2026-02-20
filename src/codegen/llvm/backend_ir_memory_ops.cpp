#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <llvm/IR/Constants.h>

#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_memory_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                     llvm::Type* i64_ty)
{
    switch (inst->getOpcode())
    {
        case ir::Opcode::Alloca:
        {
            llvm::Value* alloc = create_entry_alloca(packed_value_ty, inst->getName());
            ctx_->ir_values[inst] = alloc;
            break;
        }
        case ir::Opcode::Store:
        {
            auto ops = inst->getOperands();
            if (ops.size() < 2)
                break;
            llvm::Value* val = get_llvm_value(ops[0]);
            llvm::Value* ptr = get_llvm_value(ops[1]);

            if (val && ptr)
            {
                ctx_->builder->CreateMemCpy(ptr, llvm::MaybeAlign(8), val, llvm::MaybeAlign(8),
                                           llvm::ConstantInt::get(i64_ty, 24));
            }
            break;
        }
        case ir::Opcode::Load:
        {
            auto ops = inst->getOperands();
            if (ops.size() < 1)
                break;
            llvm::Value* ptr = get_llvm_value(ops[0]);

            if (ptr)
            {
                llvm::Value* dest = create_entry_alloca(packed_value_ty);
                ctx_->builder->CreateMemCpy(dest, llvm::MaybeAlign(8), ptr, llvm::MaybeAlign(8),
                                           llvm::ConstantInt::get(i64_ty, 24));
                ctx_->ir_values[inst] = dest;
            }
            break;
        }
        default:
            break;
    }
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
