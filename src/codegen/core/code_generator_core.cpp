/**
 * @file code_generator_core.cpp
 * @brief Core code generation logic: main entry point and visitor dispatch.
 */

#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_type.h"
#include "druk/parser/ast/type.hpp"

namespace druk::codegen
{

CodeGenerator::CodeGenerator(ir::Module& module, util::ErrorHandler& errors,
                             std::string_view source)
    : module_(module), errors_(errors), source_(source)
{
    variables_stack_.emplace_back();
}

bool CodeGenerator::generate(const std::vector<parser::ast::Stmt*>& statements)
{
    auto funcTy     = ir::Type::getVoidTy();
    auto mainFunc   = std::make_unique<ir::Function>("main", funcTy, &module_);
    auto entryBlock = std::make_unique<ir::BasicBlock>("entry", mainFunc.get());

    auto* entryBlockPtr = entryBlock.get();
    builder_.setInsertPoint(entryBlockPtr);
    mainFunc->addBasicBlock(std::move(entryBlock));

    // Pass 1: Register all function signatures
    for (auto* stmt : statements)
    {
        if (stmt->kind == parser::ast::NodeKind::Function)
        {
            auto* funcDecl = static_cast<parser::ast::FuncDecl*>(stmt);
            auto  funcName = std::string(funcDecl->name.text(source_));
            // For now, assume all functions return i64 (PackedValue)
            auto  funcTy   = ir::Type::getInt64Ty();
            auto  func     = std::make_unique<ir::Function>(funcName, funcTy, &module_);
            
            for (uint32_t i = 0; i < funcDecl->paramCount; ++i)
            {
                auto paramName = std::string(funcDecl->params[i].name.text(source_));
                auto param     = std::make_unique<ir::Parameter>(paramName, ir::Type::getInt64Ty(), i);
                func->addParameter(std::move(param));
            }

            functions_[funcName] = func.get();
            module_.addFunction(std::move(func));
        }
    }

    // Pass 2: Generate code for all statements
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

void CodeGenerator::visit(parser::ast::Type* type)
{
    if (type)
        type->accept(this);
}

void CodeGenerator::visitBlock(parser::ast::BlockStmt* stmt)
{
    variables_stack_.emplace_back();
    for (uint32_t i = 0; i < stmt->count; ++i)
    {
        visit(stmt->statements[i]);
    }
    variables_stack_.pop_back();
}

void CodeGenerator::visitGrouping(parser::ast::GroupingExpr* expr)
{
    visit(expr->expression);
}

}  // namespace druk::codegen
