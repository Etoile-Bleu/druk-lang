#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"
#include "druk/parser/ast/match.hpp"

namespace druk::codegen
{

void CodeGenerator::visitMatch(parser::ast::MatchStmt* stmt)
{
    visit(stmt->expression);
    auto* val = lastValue_;
    if (!val)
        return;
    auto* parentFunc = builder_.getInsertBlock()->getParent();
    auto  exitBlock  = std::make_unique<ir::BasicBlock>("match.exit", parentFunc);
    auto* exitPtr    = exitBlock.get();
    for (uint32_t i = 0; i < stmt->armCount; ++i)
    {
        auto& arm     = stmt->arms[i];
        auto  armBody = std::make_unique<ir::BasicBlock>("match.arm", parentFunc);
        auto  nextArm = std::make_unique<ir::BasicBlock>("match.next", parentFunc);
        auto* armPtr  = armBody.get();
        auto* nextPtr = nextArm.get();
        if (arm.pattern)
            visit(arm.pattern);
        else
            lastValue_ = nullptr;
        auto* patternVal = lastValue_;
        if (patternVal)
        {
            auto* cond = builder_.createEqual(val, patternVal);
            builder_.createCondBranch(cond, armPtr, nextPtr);
        }
        else
            builder_.createBranch(armPtr);
        parentFunc->addBasicBlock(std::move(armBody));
        parentFunc->addBasicBlock(std::move(nextArm));
        builder_.setInsertPoint(armPtr);
        visit(arm.body);
        if (!armPtr->hasTerminator())
            builder_.createBranch(exitPtr);
        builder_.setInsertPoint(nextPtr);
    }
    builder_.createBranch(exitPtr);
    parentFunc->addBasicBlock(std::move(exitBlock));
    builder_.setInsertPoint(exitPtr);
}

}  // namespace druk::codegen
