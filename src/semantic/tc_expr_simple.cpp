#include "druk/parser/ast/expr.hpp"
#include "type_checker.hpp"

namespace druk::semantic
{

void TypeChecker::visitLiteral(parser::ast::LiteralExpr* expr)
{
    if (expr->token.type == lexer::TokenType::Number)
        currentType_ = Type::makeInt();
    else if (expr->token.type == lexer::TokenType::String)
        currentType_ = Type::makeString();
    else if (expr->token.type == lexer::TokenType::KwNil)
        currentType_ = Type::makeVoid();
    else
        currentType_ = Type::makeBool();
    expr->type = currentType_;
}

void TypeChecker::visitUnwrapExpr(parser::ast::UnwrapExpr* expr)
{
    Type operandType = analyze(expr->operand);
    if (operandType.kind != TypeKind::Option && operandType.kind != TypeKind::Error)
    {
        error(expr->token, "Cannot unwrap a non-optional type.");
        currentType_ = Type::makeError();
    }
    else if (operandType.kind == TypeKind::Option)
    {
        warn(expr->token,
             "Unwrapping an option is technically unsafe. Consider using a match statement.");
        currentType_ = *operandType.elementType;
    }
    else
    {
        currentType_ = Type::makeError();
    }
    expr->type = currentType_;
}

void TypeChecker::visitVariable(parser::ast::VariableExpr* expr)
{
    if (auto* sym = table_.resolve(std::string(expr->name.text(source_))))
    {
        currentType_ = sym->type;
    }
    else
    {
        currentType_ = Type::makeError();
    }
    expr->type = currentType_;
}

void TypeChecker::visitGrouping(parser::ast::GroupingExpr* expr)
{
    currentType_ = analyze(expr->expression);
    expr->type   = currentType_;
}

void TypeChecker::visitUnary(parser::ast::UnaryExpr* expr)
{
    currentType_ = analyze(expr->right);
    expr->type   = currentType_;
}

void TypeChecker::visitLogical(parser::ast::LogicalExpr* expr)
{
    analyze(expr->left);
    analyze(expr->right);
    currentType_ = Type::makeBool();
    expr->type   = currentType_;
}

void TypeChecker::visitBinary(parser::ast::BinaryExpr* expr)
{
    Type left  = analyze(expr->left);
    Type right = analyze(expr->right);
    if (left == Type::makeInt() && right == Type::makeInt())
    {
        if (expr->token.type == lexer::TokenType::EqualEqual ||
            expr->token.type == lexer::TokenType::BangEqual ||
            expr->token.type == lexer::TokenType::Less ||
            expr->token.type == lexer::TokenType::LessEqual ||
            expr->token.type == lexer::TokenType::Greater ||
            expr->token.type == lexer::TokenType::GreaterEqual)
        {
            currentType_ = Type::makeBool();
        }
        else
        {
            currentType_ = Type::makeInt();
        }
    }
    else
    {
        currentType_ = Type::makeError();
    }
}

void TypeChecker::visitInterpolatedStringExpr(parser::ast::InterpolatedStringExpr* expr)
{
    for (uint32_t i = 0; i < expr->count; ++i)
    {
        analyze(expr->parts[i]);
        // We will allow any type to be stringified at runtime
    }

    currentType_ = Type::makeString();
    expr->type   = currentType_;
}

}  // namespace druk::semantic
