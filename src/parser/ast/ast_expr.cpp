#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/lambda.hpp"
#include "druk/parser/ast/visitor.hpp"


namespace druk::parser::ast
{

void Expr::accept(Visitor* v)
{
    v->visit(this);
}

void BinaryExpr::accept(Visitor* v)
{
    v->visitBinary(this);
}

void UnaryExpr::accept(Visitor* v)
{
    v->visitUnary(this);
}

void LiteralExpr::accept(Visitor* v)
{
    v->visitLiteral(this);
}

void VariableExpr::accept(Visitor* v)
{
    v->visitVariable(this);
}

void GroupingExpr::accept(Visitor* v)
{
    v->visitGrouping(this);
}

void AssignmentExpr::accept(Visitor* v)
{
    v->visitAssignment(this);
}

void CallExpr::accept(Visitor* v)
{
    v->visitCall(this);
}

void LogicalExpr::accept(Visitor* v)
{
    v->visitLogical(this);
}

void ArrayLiteralExpr::accept(Visitor* v)
{
    v->visitArrayLiteral(this);
}

void IndexExpr::accept(Visitor* v)
{
    v->visitIndex(this);
}

void StructLiteralExpr::accept(Visitor* v)
{
    v->visitStructLiteral(this);
}

void MemberAccessExpr::accept(Visitor* v)
{
    v->visitMemberAccess(this);
}

void LambdaExpr::accept(Visitor* v)
{
    v->visitLambda(this);
}

}  // namespace druk::parser::ast
