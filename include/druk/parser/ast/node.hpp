#pragma once

#include "druk/lexer/token.hpp"
#include "druk/parser/ast/kinds.hpp"


namespace druk {

struct Node {
  NodeKind kind;
  Token
      token; // Representative token for error reporting (e.g. 'if', identifier)

  // Base members could go here.
  // For tagged union approach, we might put everything in a big union,
  // or use inheritance with manual layout if we want strict memory control.
  // Given the constraints and C++20, simple structs + Allocator is fine.
  // We will use composition/pointers in the specialized structs.
};

} // namespace druk
