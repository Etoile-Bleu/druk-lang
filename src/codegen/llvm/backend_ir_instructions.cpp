#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <llvm/IR/Constants.h>

#include "druk/codegen/jit/jit_runtime.h"
#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_instruction(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                      llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
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
        case ir::Opcode::Print:
        {
            auto ops = inst->getOperands();
            if (ops.empty())
                break;
            llvm::Value* val = get_llvm_value(ops[0]);
            if (val)
            {
                ctx_->builder->CreateCall(
                    ctx_->module->getOrInsertFunction(
                        "druk_jit_print",
                        llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                               {packed_ptr_ty}, false)),
                    {val});
            }
            break;
        }
        default:
            compile_control_flow(inst, packed_value_ty, packed_ptr_ty, i64_ty);
            break;
    }
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
