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
  ExpressionStmt,

  // Expressions
  Binary,
  Unary,
  Literal,
  Grouping,
  VariableExpr,
  Call,
  Assignment,
  Logical
};

} // namespace druk
