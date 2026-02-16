#pragma once

#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/node.hpp"

namespace druk {

struct Stmt : Node {};

struct BlockStmt : Stmt {
  Stmt **statements;
  uint32_t count;
};

struct IfStmt : Stmt {
  Expr *condition;
  Stmt *then_branch;
  Stmt *else_branch; // Nullable
};

struct LoopStmt : Stmt {
  Expr *condition;
  Stmt *body;
};

struct ReturnStmt : Stmt {
  Expr *value; // Nullable
};

struct ExpressionStmt : Stmt {
  Expr *expression;
};

struct PrintStmt : Stmt {
  Expr *expression;
};

struct VarDecl : Stmt {
  Token name;
  Expr *initializer; // Nullable
  Token type_token;  // Explicit type from source (KwNumber, KwString, etc.)
};

struct FuncDecl : Stmt {
  Token name;
  Token *params; // Array of Name+Type tokens? simpler for now
  uint32_t param_count;
  Stmt *body; // BlockStmt
};

} // namespace druk
