#include "druk/parser/ast/match.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "type_checker.hpp"


namespace druk::semantic
{

void TypeChecker::visitIf(parser::ast::IfStmt* stmt)
{
    if (analyze(stmt->condition) != Type::makeBool())
    {
    }
    check(stmt->thenBranch);
    if (stmt->elseBranch)
        check(stmt->elseBranch);
}

void TypeChecker::visitLoop(parser::ast::LoopStmt* stmt)
{
    if (analyze(stmt->condition) != Type::makeBool())
    {
    }
    check(stmt->body);
}

void TypeChecker::visitWhile(parser::ast::WhileStmt* stmt)
{
    check(stmt->condition);
    check(stmt->body);
}

void TypeChecker::visitFor(parser::ast::ForStmt* stmt)
{
    table_.enterScope();
    if (stmt->init)
        check(stmt->init);
    if (stmt->condition)
        check(stmt->condition);
    if (stmt->step)
        check(stmt->step);
    check(stmt->body);
    table_.exitScope();
}

void TypeChecker::visitMatch(parser::ast::MatchStmt* stmt)
{
    Type exprType   = analyze(stmt->expression);
    bool hasDefault = false;
    for (uint32_t i = 0; i < stmt->armCount; ++i)
    {
        if (stmt->arms[i].pattern)
        {
            Type patType = analyze(stmt->arms[i].pattern);
            if (patType != exprType)
            {
            }
        }
        else
        {
            hasDefault = true;
        }
        check(stmt->arms[i].body);
    }
    if (!hasDefault)
    {
    }
}

}  // namespace druk::semantic
