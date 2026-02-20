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
    else
        currentType_ = Type::makeBool();
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
    expr->type = currentType_;
}

}  // namespace druk::semantic
