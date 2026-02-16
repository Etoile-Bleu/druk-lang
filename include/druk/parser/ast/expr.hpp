#pragma once

#include "druk/parser/ast/node.hpp"

#include "druk/semantic/types.hpp"

#include "druk/codegen/value.hpp"

namespace druk {

struct Expr : Node {
  Type type = Type::Void(); // Default to Void or Error?
};

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
  Value literal_value;
  // Token in Node has the value (Number/String/Bool)
};

struct GroupingExpr : Expr {
  Expr *expression;
};

struct VariableExpr : Expr {
  Token name;
  // Token in Node is the identifier
};

struct AssignmentExpr : Expr {
  Expr *target;  // Can be VariableExpr, IndexExpr, or MemberAccessExpr
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

// Arrays
struct ArrayLiteralExpr : Expr {
  Expr **elements;
  uint32_t count;
};

struct IndexExpr : Expr {
  Expr *array;  // The array/object being indexed
  Expr *index;  // The index expression
};

// Structs
struct StructLiteralExpr : Expr {
  Token *field_names;  // Array of identifiers
  Expr **field_values; // Array of expressions
  uint32_t field_count;
};

struct MemberAccessExpr : Expr {
  Expr *object;        // The object being accessed
  Token member_name;   // Field name
};

} // namespace druk
