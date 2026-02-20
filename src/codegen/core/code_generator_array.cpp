/**
 * @file code_generator_array.cpp
 * @brief Array literal and indexing IR generation.
 */

#include "druk/codegen/core/code_generator.h"

namespace druk::codegen
{

void CodeGenerator::visitArrayLiteral(parser::ast::ArrayLiteralExpr* expr)
{
    std::vector<ir::Value*> elements;
    elements.reserve(expr->count);

    for (uint32_t i = 0; i < expr->count; ++i)
    {
        visit(expr->elements[i]);
        if (!lastValue_)
        {
            errors_.report(util::Diagnostic{
                util::DiagnosticsSeverity::Error,
                {expr->token.line, 0, expr->token.offset, expr->token.length},
                "Array element could not be evaluated.",
                ""});
            lastValue_ = nullptr;
            return;
        }
        elements.push_back(lastValue_);
    }

    lastValue_ = builder_.createBuildArray(elements);
}

void CodeGenerator::visitIndex(parser::ast::IndexExpr* expr)
{
    visit(expr->array);
    auto* array_val = lastValue_;

    visit(expr->index);
    auto* index_val = lastValue_;

    if (!array_val || !index_val)
    {
        errors_.report(util::Diagnostic{
            util::DiagnosticsSeverity::Error,
            {expr->token.line, 0, expr->token.offset, expr->token.length},
            "Index expression could not be evaluated.",
            ""});
        lastValue_ = nullptr;
        return;
    }

    lastValue_ = builder_.createIndex(array_val, index_val);
}

}  // namespace druk::codegen
