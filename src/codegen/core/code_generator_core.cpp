/**
 * @file code_generator_core.cpp
 * @brief Core code generation logic: main entry point and visitor dispatch.
 */

#include "druk/codegen/core/code_generator.h"

#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_type.h"

namespace druk::codegen
{

CodeGenerator::CodeGenerator(ir::Module& module, util::ErrorHandler& errors,
                             std::string_view source)
    : module_(module), errors_(errors), source_(source)
{
}

bool CodeGenerator::generate(const std::vector<parser::ast::Stmt*>& statements)
{
    auto funcTy     = ir::Type::getVoidTy();
    auto mainFunc   = std::make_unique<ir::Function>("main", funcTy, &module_);
    auto entryBlock = std::make_unique<ir::BasicBlock>("entry", mainFunc.get());

    auto* entryBlockPtr = entryBlock.get();
    builder_.setInsertPoint(entryBlockPtr);
    mainFunc->addBasicBlock(std::move(entryBlock));

    for (auto* stmt : statements)
    {
        visit(stmt);
    }
    
    auto* currentBlock = builder_.getInsertBlock();
    if (currentBlock && !currentBlock->hasTerminator())
    {
        builder_.createRet();
    }
    
    module_.addFunction(std::move(mainFunc));

    return !errors_.hasErrors();
}

void CodeGenerator::visit(parser::ast::Stmt* stmt)
{
    if (stmt)
        stmt->accept(this);
}

void CodeGenerator::visit(parser::ast::Expr* expr)
{
    if (expr)
        expr->accept(this);
}

void CodeGenerator::visitBlock(parser::ast::BlockStmt* stmt)
{
    for (uint32_t i = 0; i < stmt->count; ++i)
    {
        visit(stmt->statements[i]);
    }
}

void CodeGenerator::visitGrouping(parser::ast::GroupingExpr* expr)
{
    visit(expr->expression);
}

}  // namespace druk::codegen
