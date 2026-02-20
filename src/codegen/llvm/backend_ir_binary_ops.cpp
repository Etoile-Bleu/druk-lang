#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_binary_op(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                    llvm::PointerType* packed_ptr_ty)
{
    auto ops = inst->getOperands();
    if (ops.size() < 2)
        return;

    llvm::Value* lhs = get_llvm_value(ops[0]);
    llvm::Value* rhs = get_llvm_value(ops[1]);

    if (!lhs || !rhs)
        return;

    llvm::Value* res = create_entry_alloca(packed_value_ty);
    const char*  fn  = "";

    switch (inst->getOpcode())
    {
        case ir::Opcode::Add:
            fn = "druk_jit_add";
            break;
        case ir::Opcode::Sub:
            fn = "druk_jit_subtract";
            break;
        case ir::Opcode::Mul:
            fn = "druk_jit_multiply";
            break;
        case ir::Opcode::Div:
            fn = "druk_jit_divide";
            break;
        case ir::Opcode::Equal:
            fn = "druk_jit_equal";
            break;
        case ir::Opcode::NotEqual:
        {
            llvm::Value* eq_tmp = create_entry_alloca(packed_value_ty);
            ctx_->builder->CreateCall(
                ctx_->module->getOrInsertFunction(
                    "druk_jit_equal",
                    llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                            {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty},
                                            false)),
                {lhs, rhs, eq_tmp});

            ctx_->builder->CreateCall(
                ctx_->module->getOrInsertFunction(
                    "druk_jit_not",
                    llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                            {packed_ptr_ty, packed_ptr_ty}, false)),
                {eq_tmp, res});

            ctx_->ir_values[inst] = res;
            return;
        }
        case ir::Opcode::LessThan:
            fn = "druk_jit_less";
            break;
        case ir::Opcode::LessEqual:
            fn = "druk_jit_less_equal";
            break;
        case ir::Opcode::GreaterThan:
            fn = "druk_jit_greater";
            break;
        case ir::Opcode::GreaterEqual:
            fn = "druk_jit_greater_equal";
            break;
        default:
            return;
    }

    ctx_->builder->CreateCall(
        ctx_->module->getOrInsertFunction(
            fn, llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                        {packed_ptr_ty, packed_ptr_ty, packed_ptr_ty}, false)),
        {lhs, rhs, res});

    ctx_->ir_values[inst] = res;
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
