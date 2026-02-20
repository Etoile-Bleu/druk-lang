#pragma once

#include <cstdint>

#include "druk/parser/ast/stmt.hpp"


namespace druk::parser::ast
{

class Visitor;

struct MatchArm
{
    Expr* pattern;
    Stmt* body;
};

struct MatchStmt : Stmt
{
    Expr*     expression;
    MatchArm* arms;
    uint32_t  armCount;
    void      accept(Visitor* v) override;
};

}  // namespace druk::parser::ast
