#pragma once

#include "druk/parser/ast/node.hpp"

namespace druk {

struct Expr : Node {};

struct BinaryExpr : Expr {
  Expr *left;
  Expr *right;
  // Token in Node is the operator
};

struct UnaryExpr : Expr {
  Expr *right;
  // Token in Node is the operator
};

struct LiteralExpr : Expr {
  // Token in Node has the value (Number/String/Bool)
};

struct GroupingExpr : Expr {
  Expr *expression;
};

struct VariableExpr : Expr {
  // Token in Node is the identifier
};

struct AssignmentExpr : Expr {
  Token name;
  Expr *value;
};

struct CallExpr : Expr {
  Expr *callee;
  // Arguments stored as linked list or array?
  // For arena, linked list is easiest, or array ptr + count.
  Node **args;
  uint32_t arg_count;
};

struct LogicalExpr : Expr {
  Expr *left;
  Expr *right;
  // Token is op (and/or)
};

} // namespace druk
