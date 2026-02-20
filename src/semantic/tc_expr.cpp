#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/lambda.hpp"
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

void TypeChecker::visitAssignment(parser::ast::AssignmentExpr* expr)
{
    Type targetType = analyze(expr->target);
    Type valType    = analyze(expr->value);
    if (targetType != valType && valType != Type::makeError() && targetType != Type::makeError())
    {
    }
    currentType_ = targetType;
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

void TypeChecker::visitCall(parser::ast::CallExpr* expr)
{
    analyze(expr->callee);
    for (uint32_t i = 0; i < expr->argCount; ++i)
        analyze(static_cast<parser::ast::Expr*>(expr->args[i]));
    currentType_ = Type::makeInt();
    expr->type   = currentType_;
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

void TypeChecker::visitLambda(parser::ast::LambdaExpr* expr)
{
    table_.enterScope();
    for (uint32_t i = 0; i < expr->paramCount; ++i)
    {
        table_.define(std::string(expr->params[i].text(source_)),
                      {expr->params[i], Type::makeInt()});
    }
    check(expr->body);
    table_.exitScope();
    currentType_ = Type::makeFunction({}, Type::makeInt());
    expr->type   = currentType_;
}

void TypeChecker::visitArrayLiteral(parser::ast::ArrayLiteralExpr* expr)
{
    for (uint32_t i = 0; i < expr->count; ++i) analyze(expr->elements[i]);
    currentType_ = Type::makeArray(Type::makeInt());
    expr->type   = currentType_;
}

void TypeChecker::visitIndex(parser::ast::IndexExpr* expr)
{
    analyze(expr->array);
    analyze(expr->index);
    currentType_ = Type::makeInt();
    expr->type   = currentType_;
}

void TypeChecker::visitStructLiteral(parser::ast::StructLiteralExpr* expr)
{
    for (uint32_t i = 0; i < expr->fieldCount; ++i) analyze(expr->fieldValues[i]);
    currentType_ = Type::makeStruct({});
    expr->type   = currentType_;
}

void TypeChecker::visitMemberAccess(parser::ast::MemberAccessExpr* expr)
{
    analyze(expr->object);
    currentType_ = Type::makeInt();
    expr->type   = currentType_;
}

}  // namespace druk::semantic
