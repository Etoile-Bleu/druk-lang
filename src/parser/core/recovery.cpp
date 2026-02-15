#include "druk/parser/core/parser.hpp"

namespace druk {

// Helpers implementation
Token Parser::peek() const { return current_; }
Token Parser::previous() const { return previous_; }
bool Parser::is_at_end() const { return current_.kind == TokenKind::EndOfFile; }

void Parser::error(Token token, std::string_view message) {
  if (panic_mode_)
    return;
  panic_mode_ = true;

  // Report error
  errors_.report(Error{ErrorLevel::Error,
                       {token.line, 0, token.offset, token.length},
                       std::string(message),
                       ""});
}

void Parser::synchronize() {
  panic_mode_ = false;
  while (!is_at_end()) {
    if (previous_.kind == TokenKind::Semicolon)
      return;

    switch (current_.kind) {
    case TokenKind::KwFunction:
    case TokenKind::KwIf:
    case TokenKind::KwLoop:
    case TokenKind::KwReturn:
    case TokenKind::KwPrint:
      return;
    default:;
    }
    advance();
  }
}

} // namespace druk
