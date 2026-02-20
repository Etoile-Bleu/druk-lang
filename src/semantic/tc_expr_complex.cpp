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
    if (valType.kind == TypeKind::Void && targetType.kind != TypeKind::Option)
    {
        error(expr->token, "Cannot assign nil to a non-optional type.");
    }
    else if (targetType != valType && valType != Type::makeError() && targetType != Type::makeError() && valType.kind != TypeKind::Void)
    {
        if (targetType.kind == TypeKind::Option && valType == *targetType.elementType)
        {
            // Assigning 'T' to 'T?' is allowed
        }
        else
        {
            // Potential type mismatch
        }
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
        table_.define(std::string(expr->params[i].name.text(source_)),
                      {expr->params[i].name, Type::makeInt()});
    }
    check(expr->body);
    table_.exitScope();
    currentType_ = Type::makeFunction({}, Type::makeInt());
    expr->type   = currentType_;
}

void TypeChecker::visitArrayLiteral(parser::ast::ArrayLiteralExpr* expr)
{
    Type elemType = Type::makeInt();
    for (uint32_t i = 0; i < expr->count; ++i)
    {
        Type t = analyze(expr->elements[i]);
        if (i == 0) elemType = t;
    }
    currentType_ = Type::makeArray(elemType);
    expr->type   = currentType_;
}

void TypeChecker::visitIndex(parser::ast::IndexExpr* expr)
{
    Type arrType = analyze(expr->array);
    analyze(expr->index);
    if (arrType.kind == TypeKind::Array && arrType.elementType)
        currentType_ = *arrType.elementType;
    else
        currentType_ = Type::makeInt(); // Fallback or Error
    expr->type = currentType_;
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
