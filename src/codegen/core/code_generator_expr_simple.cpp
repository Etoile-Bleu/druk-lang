#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::codegen
{

void CodeGenerator::visitLiteral(parser::ast::LiteralExpr* expr)
{
    if (expr->literalValue.isInt())
        lastValue_ = new ir::ConstantInt(expr->literalValue.asInt(), ir::Type::getInt64Ty());
    else if (expr->literalValue.isBool())
        lastValue_ = new ir::ConstantBool(expr->literalValue.asBool(), ir::Type::getBoolTy());
    else if (expr->literalValue.isString())
    {
        auto charPtrTy = std::make_shared<ir::PointerType>(ir::Type::getInt8Ty());
        lastValue_ = new ir::ConstantString(std::string(expr->literalValue.asString()), charPtrTy);
    }
    else if (expr->literalValue.isNil())
    {
        lastValue_ = new ir::ConstantNil(ir::Type::getVoidTy());
    }
    else
        lastValue_ = nullptr;
}

void CodeGenerator::visitVariable(parser::ast::VariableExpr* expr)
{
    auto name = std::string(expr->name.text(source_));
    for (auto itScope = variables_stack_.rbegin(); itScope != variables_stack_.rend(); ++itScope)
    {
        auto it = itScope->find(name);
        if (it != itScope->end())
        {
            lastValue_ = builder_.createLoad(it->second);
            return;
        }
    }

    auto itFunc = functions_.find(name);
    if (itFunc != functions_.end())
        lastValue_ = itFunc->second;
    else
        lastValue_ = nullptr;
}

void CodeGenerator::visitAssignment(parser::ast::AssignmentExpr* expr)
{
    visit(expr->value);
    auto* val = lastValue_;
    if (!val)
        return;
    if (auto* varExpr = dynamic_cast<parser::ast::VariableExpr*>(expr->target))
    {
        auto name = std::string(varExpr->name.text(source_));
        for (auto itScope = variables_stack_.rbegin(); itScope != variables_stack_.rend(); ++itScope)
        {
            auto it = itScope->find(name);
            if (it != itScope->end())
            {
                builder_.createStore(val, it->second);
                lastValue_ = val;
                return;
            }
        }
    }
    if (auto* indexExpr = dynamic_cast<parser::ast::IndexExpr*>(expr->target))
    {
        visit(indexExpr->array);
        auto* array_val = lastValue_;
        visit(indexExpr->index);
        auto* index_val = lastValue_;
        if (array_val && index_val)
        {
            builder_.createIndexSet(array_val, index_val, val);
            lastValue_ = val;
            return;
        }
    }
    lastValue_ = nullptr;
}

void CodeGenerator::visitInterpolatedStringExpr(parser::ast::InterpolatedStringExpr* expr)
{
    if (expr->count == 0)
    {
        auto charPtrTy = std::make_shared<ir::PointerType>(ir::Type::getInt8Ty());
        lastValue_ = new ir::ConstantString("", charPtrTy);
        return;
    }

    visit(expr->parts[0]);
    ir::Value* result = lastValue_;
    
    // First part handles literals directly but if not we should ToString it
    // Wait, first part might be an expression technically, though parser logic usually starts with literal
    result = builder_.createToString(result);

    for (uint32_t i = 1; i < expr->count; ++i)
    {
        visit(expr->parts[i]);
        ir::Value* part_val = lastValue_;
        part_val = builder_.createToString(part_val);
        
        result = builder_.createStringConcat(result, part_val);
    }
    
    lastValue_ = result;
}

void CodeGenerator::visitUnwrapExpr(parser::ast::UnwrapExpr* expr)
{
    visit(expr->operand);
    if (!lastValue_)
        return;
    lastValue_ = builder_.createUnwrap(lastValue_);
}

}  // namespace druk::codegen
