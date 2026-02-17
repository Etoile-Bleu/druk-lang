/**
 * @file code_generator_func.cpp
 * @brief Function IR generation: function definitions and calls.
 */

#include "druk/codegen/core/code_generator.h"

#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::codegen
{

void CodeGenerator::visitFunc(parser::ast::FuncDecl* stmt)
{
    auto funcTy = ir::Type::getVoidTy();
    auto funcName = std::string(stmt->name.text(source_));
    auto func   = std::make_unique<ir::Function>(funcName, funcTy, &module_);
    auto* funcPtr = func.get();

    // Save current context
    auto* savedInsertBlock = builder_.getInsertBlock();
    auto* prevFunc = currentFunction_;
    std::unordered_map<std::string, ir::Value*> prevVariables = std::move(variables_);

    for (uint32_t i = 0; i < stmt->paramCount; ++i)
    {
        auto paramName = std::string(stmt->params[i].text(source_));
        auto param = std::make_unique<ir::Parameter>(paramName, ir::Type::getInt64Ty(), i);
        funcPtr->addParameter(std::move(param));
    }

    functions_[funcName] = funcPtr;
    currentFunction_ = funcPtr;
    variables_.clear();

    auto entryBlock     = std::make_unique<ir::BasicBlock>("entry", funcPtr);
    auto* entryBlockPtr = entryBlock.get();
    funcPtr->addBasicBlock(std::move(entryBlock));

    builder_.setInsertPoint(entryBlockPtr);

    for (uint32_t i = 0; i < stmt->paramCount; ++i)
    {
        auto* param   = funcPtr->getParameter(i);
        auto* alloca  = builder_.createAlloca(ir::Type::getInt64Ty());
        auto* loadVal = builder_.createLoad(param);
        builder_.createStore(loadVal, alloca);
        auto paramName = std::string(stmt->params[i].text(source_));
        variables_[paramName] = alloca;
    }

    visit(stmt->body);

    if (entryBlockPtr && !entryBlockPtr->hasTerminator())
    {
        builder_.createRet();
    }

    module_.addFunction(std::move(func));

    // Restore context
    variables_      = std::move(prevVariables);
    currentFunction_ = prevFunc;
    builder_.setInsertPoint(savedInsertBlock);
}

void CodeGenerator::visitCall(parser::ast::CallExpr* expr)
{
    auto* varExpr = static_cast<parser::ast::VariableExpr*>(expr->callee);
    if (!varExpr)
    {
        lastValue_ = nullptr;
        return;
    }

    auto funcName = std::string(varExpr->name.text(source_));
    auto it = functions_.find(funcName);
    if (it == functions_.end())
    {
        lastValue_ = nullptr;
        return;
    }

    auto* func = it->second;

    std::vector<ir::Value*> args;
    for (uint32_t i = 0; i < expr->argCount; ++i)
    {
        auto* argExpr = static_cast<parser::ast::Expr*>(expr->args[i]);
        visit(argExpr);
        if (lastValue_)
            args.push_back(lastValue_);
    }

    lastValue_ = builder_.createCall(func, args);
}

}  // namespace druk::codegen
