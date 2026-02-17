/**
 * @file code_generator_stmt.cpp
 * @brief Statement IR generation: if, loop, return, print, variable declarations.
 */

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

    auto thenBlock  = std::make_unique<ir::BasicBlock>("then", parentFunc);
    auto elseBlock  = std::make_unique<ir::BasicBlock>("else", parentFunc);
    auto mergeBlock = std::make_unique<ir::BasicBlock>("merge", parentFunc);

    auto* thenPtr  = thenBlock.get();
    auto* elsePtr  = elseBlock.get();
    auto* mergePtr = mergeBlock.get();

    parentFunc->addBasicBlock(std::move(thenBlock));
    parentFunc->addBasicBlock(std::move(elseBlock));
    parentFunc->addBasicBlock(std::move(mergeBlock));

    builder_.createCondBranch(cond, thenPtr, elsePtr);

    builder_.setInsertPoint(thenPtr);
    visit(stmt->thenBranch);
    if (!thenPtr->hasTerminator())
    {
        builder_.createBranch(mergePtr);
    }

    builder_.setInsertPoint(elsePtr);
    if (stmt->elseBranch)
        visit(stmt->elseBranch);
    if (!elsePtr->hasTerminator())
    {
        builder_.createBranch(mergePtr);
    }

    builder_.setInsertPoint(mergePtr);
}

void CodeGenerator::visitLoop(parser::ast::LoopStmt* stmt)
{
    auto* parentFunc = builder_.getInsertBlock()->getParent();

    auto headerBlock = std::make_unique<ir::BasicBlock>("loop.header", parentFunc);
    auto bodyBlock   = std::make_unique<ir::BasicBlock>("loop.body", parentFunc);
    auto exitBlock   = std::make_unique<ir::BasicBlock>("loop.exit", parentFunc);

    auto* headerPtr = headerBlock.get();
    auto* bodyPtr   = bodyBlock.get();
    auto* exitPtr   = exitBlock.get();

    parentFunc->addBasicBlock(std::move(headerBlock));
    parentFunc->addBasicBlock(std::move(bodyBlock));
    parentFunc->addBasicBlock(std::move(exitBlock));

    builder_.createBranch(headerPtr);
    builder_.setInsertPoint(headerPtr);

    visit(stmt->condition);

    if (stmt->condition)
    {
        builder_.createCondBranch(lastValue_, bodyPtr, exitPtr);
    }
    else
    {
        builder_.createBranch(bodyPtr);
    }

    builder_.setInsertPoint(bodyPtr);
    visit(stmt->body);
    builder_.createBranch(headerPtr);

    builder_.setInsertPoint(exitPtr);
}

void CodeGenerator::visitReturn(parser::ast::ReturnStmt* stmt)
{
    if (stmt->value)
    {
        visit(stmt->value);
        auto* retVal = lastValue_;
        if (retVal)
            builder_.createRet(retVal);
    }
    else
    {
        builder_.createRet();
    }
}

void CodeGenerator::visitPrint(parser::ast::PrintStmt* stmt)
{
    visit(stmt->expression);
    if (lastValue_)
        builder_.createPrint(lastValue_);
}

void CodeGenerator::visitVar(parser::ast::VarDecl* stmt)
{
    auto* alloca = builder_.createAlloca(ir::Type::getInt64Ty());
    auto name = std::string(stmt->name.text(source_));
    variables_[name] = alloca;

    if (stmt->initializer)
    {
        visit(stmt->initializer);
        if (lastValue_)
            builder_.createStore(lastValue_, alloca);
    }
}

void CodeGenerator::visitExprStmt(parser::ast::ExpressionStmt* stmt)
{
    visit(stmt->expression);
}

}  // namespace druk::codegen
