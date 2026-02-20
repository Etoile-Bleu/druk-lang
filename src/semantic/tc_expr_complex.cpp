#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/lambda.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "type_checker.hpp"

namespace druk::semantic
{

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

void TypeChecker::visitCall(parser::ast::CallExpr* expr)
{
    analyze(expr->callee);
    for (uint32_t i = 0; i < expr->argCount; ++i)
        analyze(static_cast<parser::ast::Expr*>(expr->args[i]));
    currentType_ = Type::makeInt();
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
