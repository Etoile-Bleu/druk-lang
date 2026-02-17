#pragma once

#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/node.hpp"

namespace druk::parser::ast
{

/**
 * @brief Base class for statement nodes.
 */
class Visitor;

/**
 * @brief Base class for statement nodes.
 */
struct Stmt : Node
{
    virtual void accept(Visitor* v);
};

struct BlockStmt : Stmt
{
    Stmt**   statements;
    uint32_t count;
    void     accept(Visitor* v) override;
};

struct IfStmt : Stmt
{
    Expr* condition;
    Stmt* thenBranch;
    Stmt* elseBranch;  // Nullable
    void  accept(Visitor* v) override;
};

struct LoopStmt : Stmt
{
    Expr* condition;
    Stmt* body;
    void  accept(Visitor* v) override;
};

struct ReturnStmt : Stmt
{
    Expr* value;  // Nullable
    void  accept(Visitor* v) override;
};

struct ExpressionStmt : Stmt
{
    Expr* expression;
    void  accept(Visitor* v) override;
};

struct PrintStmt : Stmt
{
    Expr* expression;
    void  accept(Visitor* v) override;
};

struct VarDecl : Stmt
{
    lexer::Token name;
    Expr*        initializer;  // Nullable
    lexer::Token typeToken;
    void         accept(Visitor* v) override;
};

struct FuncDecl : Stmt
{
    lexer::Token  name;
    lexer::Token* params;
    uint32_t      paramCount;
    Stmt*         body;  // Should be BlockStmt
    void          accept(Visitor* v) override;
};

}  // namespace druk::parser::ast
