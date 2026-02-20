#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::codegen
{

void CodeGenerator::visitIf(parser::ast::IfStmt* stmt)
{
    visit(stmt->condition);
    auto* cond = lastValue_;
    if (!cond)
        return;
    auto* parentFunc = builder_.getInsertBlock()->getParent();
    auto  thenBlock  = std::make_unique<ir::BasicBlock>("then", parentFunc);
    auto  elseBlock  = std::make_unique<ir::BasicBlock>("else", parentFunc);
    auto* thenPtr    = thenBlock.get();
    auto* elsePtr    = elseBlock.get();
    parentFunc->addBasicBlock(std::move(thenBlock));
    parentFunc->addBasicBlock(std::move(elseBlock));
    builder_.createCondBranch(cond, thenPtr, elsePtr);
    builder_.setInsertPoint(thenPtr);
    visit(stmt->thenBranch);
    auto* currentThen = builder_.getInsertBlock();

    builder_.setInsertPoint(elsePtr);
    if (stmt->elseBranch)
        visit(stmt->elseBranch);
    auto* currentElse = builder_.getInsertBlock();

    auto  mergeBlock = std::make_unique<ir::BasicBlock>("merge", parentFunc);
    auto* mergePtr   = mergeBlock.get();
    parentFunc->addBasicBlock(std::move(mergeBlock));

    if (!currentThen->hasTerminator())
    {
        builder_.setInsertPoint(currentThen);
        builder_.createBranch(mergePtr);
    }
    if (!currentElse->hasTerminator())
    {
        builder_.setInsertPoint(currentElse);
        builder_.createBranch(mergePtr);
    }
    builder_.setInsertPoint(mergePtr);
}

void CodeGenerator::visitReturn(parser::ast::ReturnStmt* stmt)
{
    if (stmt->value)
    {
        visit(stmt->value);
        if (lastValue_)
            builder_.createRet(lastValue_);
    }
    else
    {
        builder_.createRet();
    }
}

}  // namespace druk::codegen
