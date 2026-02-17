/**
 * @file code_generator_stubs.cpp
 * @brief Stub implementations for unimplemented features.
 */

#include "druk/codegen/core/code_generator.h"

namespace druk::codegen
{

void CodeGenerator::visitArrayLiteral(parser::ast::ArrayLiteralExpr* expr)
{
    lastValue_ = nullptr;
}

void CodeGenerator::visitStructLiteral(parser::ast::StructLiteralExpr* expr)
{
    lastValue_ = nullptr;
}

void CodeGenerator::visitMemberAccess(parser::ast::MemberAccessExpr* expr)
{
    lastValue_ = nullptr;
}

void CodeGenerator::visitIndex(parser::ast::IndexExpr* expr)
{
    lastValue_ = nullptr;
}

void CodeGenerator::visitLogical(parser::ast::LogicalExpr* expr)
{
    lastValue_ = nullptr;
}

}  // namespace druk::codegen
