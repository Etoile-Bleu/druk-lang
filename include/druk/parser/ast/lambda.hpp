#pragma once

#include <cstdint>

#include "druk/lexer/token.hpp"
#include "druk/parser/ast/expr.hpp"


namespace druk::parser::ast
{

struct Stmt;
class Visitor;

struct LambdaExpr : Expr
{
    lexer::Token* params;
    uint32_t      paramCount;
    Stmt*         body;
    void          accept(Visitor* v) override;
};

}  // namespace druk::parser::ast
