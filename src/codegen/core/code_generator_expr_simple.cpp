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
    else
        lastValue_ = nullptr;
}

void CodeGenerator::visitVariable(parser::ast::VariableExpr* expr)
{
    auto name = std::string(expr->name.text(source_));
    auto it   = variables_.find(name);
    if (it != variables_.end())
        lastValue_ = builder_.createLoad(it->second);
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
        auto it   = variables_.find(name);
        if (it != variables_.end())
        {
            builder_.createStore(val, it->second);
            lastValue_ = val;
            return;
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

}  // namespace druk::codegen
