#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <iostream>
#include <llvm/IR/Constants.h>

#include "druk/codegen/jit/jit_runtime.h"
#include "druk/ir/ir_instruction.h"

namespace druk::codegen
{

void LLVMBackend::compile_control_flow(ir::Instruction* inst, llvm::StructType* packed_value_ty,
                                       llvm::PointerType* packed_ptr_ty, llvm::Type* i64_ty)
{
    switch (inst->getOpcode())
    {
        case ir::Opcode::Branch:
        {
            if (auto* br = dynamic_cast<ir::BranchInst*>(inst))
            {
                ctx_->builder->CreateBr(ctx_->ir_blocks[br->getDest()]);
            }
            break;
        }
        case ir::Opcode::ConditionalBranch:
        {
            if (auto* br = dynamic_cast<ir::CondBranchInst*>(inst))
            {
                auto ops = br->getOperands();
                if (ops.empty() || !get_llvm_value(ops[0]))
                    break;
                
                llvm::Value* condPacked = get_llvm_value(ops[0]);
                llvm::Value* condBool   = ctx_->builder->CreateCall(
                    ctx_->module->getOrInsertFunction(
                        "druk_jit_value_as_bool",
                        llvm::FunctionType::get(llvm::Type::getInt1Ty(*ctx_->context),
                                                {packed_ptr_ty}, false)),
                    {condPacked});

                ctx_->builder->CreateCondBr(condBool, ctx_->ir_blocks[br->getTrueDest()],
                                            ctx_->ir_blocks[br->getFalseDest()]);
            }
            break;
        }
        case ir::Opcode::Return:
        {
            auto ops = inst->getOperands();
            if (!ops.empty() && ctx_->ret_out)
            {
                llvm::Value* retVal = get_llvm_value(ops[0]);
                ctx_->builder->CreateMemCpy(ctx_->ret_out, llvm::MaybeAlign(8), retVal,
                                           llvm::MaybeAlign(8),
                                           llvm::ConstantInt::get(i64_ty, 24));
            }
            else if (ctx_->ret_out)
            {
                ctx_->builder->CreateCall(
                    ctx_->module->getOrInsertFunction(
                        "druk_jit_value_nil",
                        llvm::FunctionType::get(llvm::Type::getVoidTy(*ctx_->context),
                                               {packed_ptr_ty}, false)),
                    {ctx_->ret_out});
            }

            ctx_->builder->CreateRetVoid();
            break;
        }
        case ir::Opcode::Call:
        {
            if (auto* callInst = dynamic_cast<ir::CallInst*>(inst))
            {
                ir::Function* callee = callInst->getCallee();
                if (!callee)
                    break;
                
                // Get LLVM function
                auto it = ctx_->ir_functions.find(callee);
                if (it == ctx_->ir_functions.end())
                    break;
                    
                llvm::Function* llvmFunc = it->second;
                
                // Prepare arguments
                std::vector<llvm::Value*> args;
                
                // First argument is the return value
                llvm::Value* retVal = create_entry_alloca(packed_value_ty);
                args.push_back(retVal);
                
                // Add function parameters
                auto operands = callInst->getOperands();
                for (auto* operand : operands)
                {
                    llvm::Value* argVal = get_llvm_value(operand);
                    if (argVal)
                    {
                        args.push_back(argVal);
                    }
                }
                
                // Create call
                ctx_->builder->CreateCall(llvmFunc, args);
                
                // Store result
                ctx_->ir_values[inst] = retVal;
            }
            break;
        }
        default:
            break;
    }
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
