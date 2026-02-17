#include "druk/lexer/lexer.hpp"

namespace druk::lexer {

Token Lexer::scanString() {
  while (peek() != '"' && currentOffset_ < source_.length()) {
    if (peek() == '\n') line_++;
    advance();
  }

  if (currentOffset_ >= source_.length()) {
    errors_.report(util::Diagnostic{
        util::DiagnosticsSeverity::Error, {line_, 0, 0, 0}, "Unterminated string.", ""});
    return makeErrorToken("Unterminated string.");
  }

  advance();
  return makeToken(TokenType::String);
}

} // namespace druk::lexer
