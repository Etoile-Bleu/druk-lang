/**
 * @file code_generator_stubs.cpp
 * @brief Stub implementations for unimplemented features.
 */

#include "druk/codegen/core/code_generator.h"

namespace druk::codegen
{

void CodeGenerator::visitStructLiteral(parser::ast::StructLiteralExpr* expr)
{
    lastValue_ = nullptr;
}

void CodeGenerator::visitMemberAccess(parser::ast::MemberAccessExpr* expr)
{
    lastValue_ = nullptr;
}

void CodeGenerator::visitLogical(parser::ast::LogicalExpr* expr)
{
    lastValue_ = nullptr;
}

void CodeGenerator::visitBuiltinType(parser::ast::BuiltinType* type) {}
void CodeGenerator::visitArrayType(parser::ast::ArrayType* type) {}
void CodeGenerator::visitFunctionType(parser::ast::FunctionType* type) {}

}  // namespace druk::codegen
