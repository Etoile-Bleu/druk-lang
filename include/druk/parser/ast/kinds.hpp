#pragma once

#include <cstdint>

namespace druk {

enum class NodeKind : uint8_t {
  // Declarations
  Function,
  Variable,

  // Statements
  Block,
  If,
  Loop,
  Return,
  Print,
  ExpressionStmt,

  // Expressions
  Binary,
  Unary,
  Literal,
  Grouping,
  VariableExpr,
  Call,
  Assignment,
  Logical,
  ArrayLiteral,
  Index,
  StructLiteral,
  MemberAccess
};

} // namespace druk
