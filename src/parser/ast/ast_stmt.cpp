#include "druk/parser/ast/match.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/parser/ast/visitor.hpp"


namespace druk::parser::ast
{

void Stmt::accept(Visitor* v)
{
    v->visit(this);
}

void BlockStmt::accept(Visitor* v)
{
    v->visitBlock(this);
}

void IfStmt::accept(Visitor* v)
{
    v->visitIf(this);
}

void LoopStmt::accept(Visitor* v)
{
    v->visitLoop(this);
}

void WhileStmt::accept(Visitor* v)
{
    v->visitWhile(this);
}

void ForStmt::accept(Visitor* v)
{
    v->visitFor(this);
}

void ReturnStmt::accept(Visitor* v)
{
    v->visitReturn(this);
}

void PrintStmt::accept(Visitor* v)
{
    v->visitPrint(this);
}

void ExpressionStmt::accept(Visitor* v)
{
    v->visitExprStmt(this);
}

void VarDecl::accept(Visitor* v)
{
    v->visitVar(this);
}

void FuncDecl::accept(Visitor* v)
{
    v->visitFunc(this);
}

void MatchStmt::accept(Visitor* v)
{
    v->visitMatch(this);
}

}  // namespace druk::parser::ast
