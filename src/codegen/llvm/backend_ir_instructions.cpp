#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"
#include "druk/ir/ir_instruction.h"


namespace druk::codegen
{

void LLVMBackend::compile_instruction(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                      llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
{
    switch (inst->getOpcode())
    {
        case ir::Opcode::Alloca:
        case ir::Opcode::Store:
        case ir::Opcode::Load:
        {
            compile_memory_ops(inst, packed_value_ty, i64_ty);
            break;
        }
        case ir::Opcode::Add:
        case ir::Opcode::Sub:
        case ir::Opcode::Mul:
        case ir::Opcode::Div:
        case ir::Opcode::Equal:
        case ir::Opcode::NotEqual:
        case ir::Opcode::LessThan:
        case ir::Opcode::LessEqual:
        case ir::Opcode::GreaterThan:
        case ir::Opcode::GreaterEqual:
        {
            compile_binary_op(inst, packed_value_ty, packed_ptr_ty);
            break;
        }
        case ir::Opcode::BuildArray:
        case ir::Opcode::IndexGet:
        case ir::Opcode::IndexSet:
        case ir::Opcode::Len:
        {
            compile_array_ops(inst, packed_value_ty, packed_ptr_ty, i64_ty);
            break;
        }
        case ir::Opcode::Call:
        {
            compile_call_op(inst, packed_ptr_ty);
            break;
        }
        case ir::Opcode::DynamicCall:
        {
            compile_dynamic_call_op(inst, packed_value_ty, packed_ptr_ty, i64_ty);
            break;
        }
        case ir::Opcode::Print:
        {
            compile_print_op(inst, packed_ptr_ty);
            break;
        }
        default:
            compile_control_flow(inst, packed_value_ty, packed_ptr_ty, i64_ty);
            break;
    }
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
