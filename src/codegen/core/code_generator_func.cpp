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
    auto  funcName = std::string(stmt->name.text(source_));
    ir::Function* funcPtr = nullptr;
    
    auto it = functions_.find(funcName);
    if (it != functions_.end())
    {
        funcPtr = it->second;
    }
    else
    {
        // This shouldn't happen for top-level functions in two-pass,
        // but might for nested functions if we don't pre-register them.
        auto funcTy = ir::Type::getInt64Ty();
        auto func   = std::make_unique<ir::Function>(funcName, funcTy, &module_);
        funcPtr     = func.get();
        
        for (uint32_t i = 0; i < stmt->paramCount; ++i)
        {
            auto paramName = std::string(stmt->params[i].name.text(source_));
            auto param     = std::make_unique<ir::Parameter>(paramName, ir::Type::getInt64Ty(), i);
            funcPtr->addParameter(std::move(param));
        }
        functions_[funcName] = funcPtr;
        module_.addFunction(std::move(func));
    }

    // Save current context
    auto*         savedInsertBlock = builder_.getInsertBlock();
    auto*         prevFunc         = currentFunction_;

    currentFunction_ = funcPtr;
    variables_stack_.emplace_back(); // New scope

    auto  entryBlock    = std::make_unique<ir::BasicBlock>("entry", funcPtr);
    auto* entryBlockPtr = entryBlock.get();
    funcPtr->addBasicBlock(std::move(entryBlock));

    builder_.setInsertPoint(entryBlockPtr);

    for (uint32_t i = 0; i < stmt->paramCount; ++i)
    {
        auto* param   = funcPtr->getParameter(i);
        auto* alloca  = builder_.createAlloca(ir::Type::getInt64Ty());
        auto* loadVal = builder_.createLoad(param);
        builder_.createStore(loadVal, alloca);
        auto paramName                        = std::string(stmt->params[i].name.text(source_));
        variables_stack_.back()[paramName]    = alloca;
    }

    visit(stmt->body);

    auto* currentBlock = builder_.getInsertBlock();
    if (currentBlock && !currentBlock->hasTerminator())
    {
        builder_.createRet();
    }

    // Restore context
    variables_stack_.pop_back();
    currentFunction_ = prevFunc;
    builder_.setInsertPoint(savedInsertBlock);
}

}  // namespace druk::codegen
