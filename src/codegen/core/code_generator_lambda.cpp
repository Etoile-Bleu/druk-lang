#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"
#include "druk/parser/ast/lambda.hpp"

namespace druk::codegen
{

void CodeGenerator::visitLambda(parser::ast::LambdaExpr* expr)
{
    auto  funcTy     = ir::Type::getInt64Ty();
    auto  lambdaName = "lambda_" + std::to_string(lambdaCount_++);
    auto  lambdaFunc = std::make_unique<ir::Function>(lambdaName, funcTy, &module_);
    auto* lambdaPtr  = lambdaFunc.get();

    // Save current context
    auto*         savedInsertBlock = builder_.getInsertBlock();
    auto*         prevFunc         = currentFunction_;

    currentFunction_ = lambdaPtr;
    variables_stack_.emplace_back(); // New scope

    for (uint32_t i = 0; i < expr->paramCount; ++i)
    {
        auto paramName = std::string(expr->params[i].name.text(source_));
        auto param     = std::make_unique<ir::Parameter>(paramName, ir::Type::getInt64Ty(), i);
        lambdaPtr->addParameter(std::move(param));
    }

    auto  entryBlock    = std::make_unique<ir::BasicBlock>("entry", lambdaPtr);
    auto* entryBlockPtr = entryBlock.get();
    lambdaPtr->addBasicBlock(std::move(entryBlock));

    builder_.setInsertPoint(entryBlockPtr);

    for (uint32_t i = 0; i < expr->paramCount; ++i)
    {
        auto* param   = lambdaPtr->getParameter(i);
        auto* alloca  = builder_.createAlloca(ir::Type::getInt64Ty());
        auto* loadVal = builder_.createLoad(param);
        builder_.createStore(loadVal, alloca);
        auto paramName                        = std::string(expr->params[i].name.text(source_));
        variables_stack_.back()[paramName]    = alloca;
    }

    visit(expr->body);

    auto* currentBlock = builder_.getInsertBlock();
    if (currentBlock && !currentBlock->hasTerminator())
    {
        if (expr->body->kind == parser::ast::NodeKind::ExpressionStmt)
            builder_.createRet(lastValue_);
        else
            builder_.createRet();
    }

    module_.addFunction(std::move(lambdaFunc));

    // Restore context
    variables_stack_.pop_back();
    currentFunction_ = prevFunc;
    builder_.setInsertPoint(savedInsertBlock);

    lastValue_ = lambdaPtr;
}

}  // namespace druk::codegen
