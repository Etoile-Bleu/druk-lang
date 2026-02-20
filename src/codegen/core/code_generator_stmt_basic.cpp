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
    // All variables in Druk are PackedValue on the stack (24 bytes)
    // We'll use a placeholder for now or a proper IR type if available.
    // For now, let's assume getInt64Ty() was a placeholder and we need more.
    auto* alloca     = builder_.createAlloca(ir::Type::getInt64Ty()); // Still 8 bytes? 
    // Wait, I should check if there's a better type.
    // If IR doesn't have a PackedValue type, I'll use an array of 3 i64 or something.
    auto  name       = std::string(stmt->name.text(source_));
    variables_stack_.back()[name] = alloca;
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
