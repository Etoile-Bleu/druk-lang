#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::codegen
{

void CodeGenerator::visitLoop(parser::ast::LoopStmt* stmt) {}

void CodeGenerator::visitWhile(parser::ast::WhileStmt* stmt)
{
    auto* parentFunc  = builder_.getInsertBlock()->getParent();
    auto  headerBlock = std::make_unique<ir::BasicBlock>("while.header", parentFunc);
    auto  bodyBlock   = std::make_unique<ir::BasicBlock>("while.body", parentFunc);
    auto  exitBlock   = std::make_unique<ir::BasicBlock>("while.exit", parentFunc);
    auto* headerPtr   = headerBlock.get();
    auto* bodyPtr     = bodyBlock.get();
    auto* exitPtr     = exitBlock.get();
    parentFunc->addBasicBlock(std::move(headerBlock));
    parentFunc->addBasicBlock(std::move(bodyBlock));
    parentFunc->addBasicBlock(std::move(exitBlock));
    builder_.createBranch(headerPtr);
    builder_.setInsertPoint(headerPtr);
    visit(stmt->condition);
    if (lastValue_)
        builder_.createCondBranch(lastValue_, bodyPtr, exitPtr);
    else
        builder_.createBranch(exitPtr);
    builder_.setInsertPoint(bodyPtr);
    visit(stmt->body);
    builder_.createBranch(headerPtr);
    builder_.setInsertPoint(exitPtr);
}

void CodeGenerator::visitFor(parser::ast::ForStmt* stmt)
{
    auto* parentFunc = builder_.getInsertBlock()->getParent();
    if (stmt->init)
        visit(stmt->init);
    auto  headerBlock = std::make_unique<ir::BasicBlock>("for.header", parentFunc);
    auto  bodyBlock   = std::make_unique<ir::BasicBlock>("for.body", parentFunc);
    auto  stepBlock   = std::make_unique<ir::BasicBlock>("for.step", parentFunc);
    auto  exitBlock   = std::make_unique<ir::BasicBlock>("for.exit", parentFunc);
    auto* headerPtr   = headerBlock.get();
    auto* bodyPtr     = bodyBlock.get();
    auto* stepPtr     = stepBlock.get();
    auto* exitPtr     = exitBlock.get();
    parentFunc->addBasicBlock(std::move(headerBlock));
    parentFunc->addBasicBlock(std::move(bodyBlock));
    parentFunc->addBasicBlock(std::move(stepBlock));
    parentFunc->addBasicBlock(std::move(exitBlock));
    builder_.createBranch(headerPtr);
    builder_.setInsertPoint(headerPtr);
    if (stmt->condition)
    {
        visit(stmt->condition);
        if (lastValue_)
            builder_.createCondBranch(lastValue_, bodyPtr, exitPtr);
        else
            builder_.createBranch(exitPtr);
    }
    else
        builder_.createBranch(bodyPtr);
    builder_.setInsertPoint(bodyPtr);
    visit(stmt->body);
    builder_.createBranch(stepPtr);
    builder_.setInsertPoint(stepPtr);
    if (stmt->step)
        visit(stmt->step);
    builder_.createBranch(headerPtr);
    builder_.setInsertPoint(exitPtr);
}

}  // namespace druk::codegen
