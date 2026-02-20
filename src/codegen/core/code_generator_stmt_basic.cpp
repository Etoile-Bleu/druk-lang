#include "druk/codegen/core/code_generator.h"
#include "druk/ir/ir_basic_block.h"
#include "druk/ir/ir_function.h"
#include "druk/ir/ir_instruction.h"
#include "druk/ir/ir_type.h"
#include "druk/ir/ir_value.h"

namespace druk::codegen
{

void CodeGenerator::visitPrint(parser::ast::PrintStmt* stmt)
{
    visit(stmt->expression);
    if (lastValue_)
        builder_.createPrint(lastValue_);
}

void CodeGenerator::visitVar(parser::ast::VarDecl* stmt)
{
    auto* alloca     = builder_.createAlloca(ir::Type::getInt64Ty());
    auto  name       = std::string(stmt->name.text(source_));
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
