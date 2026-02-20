#pragma once

#include <cstdint>

#include "druk/lexer/token.hpp"
#include "druk/parser/ast/expr.hpp"


namespace druk::parser::ast
{

struct Stmt;
struct Type;
struct Param;
class Visitor;

struct LambdaExpr : Expr
{
    Param*        params;
    uint32_t      paramCount;
    Type*         returnType;
    Stmt*         body;
    void          accept(Visitor* v) override;
};

}  // namespace druk::parser::ast
