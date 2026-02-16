#pragma once

#include "druk/lexer/token.hpp"
#include "druk/semantic/types.hpp"

namespace druk {

struct Symbol {
  Token name;
  Type type;
  // We can add depth, offset, etc.
};

} // namespace druk
