#pragma once

#include "druk/lexer/token.hpp"
#include "druk/parser/ast/kinds.hpp"

namespace druk::parser::ast {

/**
 * @brief Base class for all AST nodes.
 */
struct Node {
  virtual ~Node() = default;

  NodeKind kind;
  lexer::Token token; // Representative token for error reporting
};

} // namespace druk::parser::ast
