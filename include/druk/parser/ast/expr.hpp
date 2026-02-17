#pragma once

#include "druk/codegen/core/value.h"
#include "druk/parser/ast/node.hpp"
#include "druk/semantic/types.hpp"

namespace druk::parser::ast
{

/**
 * @brief Base class for expression nodes.
 */
class Visitor;

/**
 * @brief Base class for expression nodes.
 */
struct Expr : Node
{
    semantic::Type type = semantic::Type::makeVoid();
    virtual void   accept(Visitor* v);
};

struct BinaryExpr : Expr
{
    void  accept(Visitor* v) override;
    Expr* left;
    Expr* right;
};

struct UnaryExpr : Expr
{
    Expr* right;
    void  accept(Visitor* v) override;
};

struct LiteralExpr : Expr
{
    codegen::Value literalValue;
    void           accept(Visitor* v) override;
};

struct GroupingExpr : Expr
{
    Expr* expression;
    void  accept(Visitor* v) override;
};

struct VariableExpr : Expr
{
    lexer::Token name;
    void         accept(Visitor* v) override;
};

struct AssignmentExpr : Expr
{
    Expr* target;
    Expr* value;
    void  accept(Visitor* v) override;
};

struct CallExpr : Expr
{
    Expr*    callee;
    Node**   args;
    uint32_t argCount;
    void     accept(Visitor* v) override;
};

struct LogicalExpr : Expr
{
    Expr* left;
    Expr* right;
    void  accept(Visitor* v) override;
};

struct ArrayLiteralExpr : Expr
{
    Expr**   elements;
    uint32_t count;
    void     accept(Visitor* v) override;
};

struct IndexExpr : Expr
{
    Expr* array;
    Expr* index;
    void  accept(Visitor* v) override;
};

struct StructLiteralExpr : Expr
{
    lexer::Token* fieldNames;
    Expr**        fieldValues;
    uint32_t      fieldCount;
    void          accept(Visitor* v) override;
};

struct MemberAccessExpr : Expr
{
    Expr*        object;
    lexer::Token memberName;
    void         accept(Visitor* v) override;
};

}  // namespace druk::parser::ast
