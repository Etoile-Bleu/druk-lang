#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_array_ops(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                    llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
{
    switch (inst->getOpcode())
    {
        case ir::Opcode::BuildArray:
            compile_array_build(inst, packed_value_ty, packed_ptr_ty, i64_ty);
            break;
        case ir::Opcode::IndexGet:
        case ir::Opcode::IndexSet:
            compile_array_index(inst, packed_value_ty, packed_ptr_ty);
            break;
        case ir::Opcode::Len:
            compile_array_len(inst, packed_value_ty, packed_ptr_ty);
            break;
        default:
            break;
    }
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
