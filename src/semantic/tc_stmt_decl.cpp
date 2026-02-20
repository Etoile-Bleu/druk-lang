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
        if (initType.kind == TypeKind::Void && expected.kind != TypeKind::Option)
        {
            error(stmt->name, "Cannot assign nil to a non-optional type.");
        }
        else if (initType != expected && initType != Type::makeError() && initType.kind != TypeKind::Void)
        {
            if (expected.kind == TypeKind::Option && initType == *expected.elementType)
            {
                // Assigning 'T' to 'T?' is allowed
            }
            else if (initType.kind == TypeKind::Array)
            {
                Type* baseType = &initType;
                while (baseType->kind == TypeKind::Array && baseType->elementType)
                {
                    baseType = baseType->elementType.get();
                }

                if (baseType->kind == expected.kind)
                {
                    expected = initType;
                }
                else
                {
                    error(stmt->name, "Type mismatch: cannot assign " + typeToString(initType) + " to " + typeToString(expected) + ".");
                }
            }
            else
            {
                error(stmt->name, "Type mismatch: cannot assign " + typeToString(initType) + " to " + typeToString(expected) + ".");
            }
        }
    }
    
    auto name = std::string(stmt->name.text(source_));
    if (auto* sym = table_.resolveLocal(name))
    {
        sym->type = expected;
    }
    else
    {
        table_.define(name, {stmt->name, expected});
    }
}

void TypeChecker::visitFunc(parser::ast::FuncDecl* stmt)
{
    table_.enterScope();
    for (uint32_t i = 0; i < stmt->paramCount; ++i)
    {
        auto name = std::string(stmt->params[i].name.text(source_));
        Type type = evaluate(stmt->params[i].type);
        if (auto* sym = table_.resolveLocal(name))
        {
            sym->type = type;
        }
        else
        {
            table_.define(name, {stmt->params[i].name, type});
        }
    }
    check(stmt->body);
    table_.exitScope();
}

}  // namespace druk::semantic
