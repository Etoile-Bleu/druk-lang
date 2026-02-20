#include "druk/parser/ast/stmt.hpp"
#include "type_checker.hpp"

namespace druk::semantic
{

void TypeChecker::visitBlock(parser::ast::BlockStmt* stmt)
{
    table_.enterScope();
    for (uint32_t i = 0; i < stmt->count; ++i) check(stmt->statements[i]);
    table_.exitScope();
}

void TypeChecker::visitReturn(parser::ast::ReturnStmt* stmt)
{
    if (stmt->value)
        analyze(stmt->value);
}

void TypeChecker::visitPrint(parser::ast::PrintStmt* stmt)
{
    analyze(stmt->expression);
}

void TypeChecker::visitExprStmt(parser::ast::ExpressionStmt* stmt)
{
    analyze(stmt->expression);
}

void TypeChecker::visitVar(parser::ast::VarDecl* stmt)
{
    Type expected = evaluate(stmt->type);

    if (stmt->initializer)
    {
        Type initType = analyze(stmt->initializer);
        if (initType != expected && initType != Type::makeError())
        {
        }
    }
    table_.define(std::string(stmt->name.text(source_)), {stmt->name, expected});
}

void TypeChecker::visitFunc(parser::ast::FuncDecl* stmt)
{
    table_.enterScope();
    for (uint32_t i = 0; i < stmt->paramCount; ++i)
    {
        table_.define(std::string(stmt->params[i].name.text(source_)),
                      {stmt->params[i].name, evaluate(stmt->params[i].type)});
    }
    check(stmt->body);
    table_.exitScope();
}

}  // namespace druk::semantic
