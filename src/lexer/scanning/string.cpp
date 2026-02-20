#include "druk/lexer/lexer.hpp"

namespace druk::lexer {

Token Lexer::scanString(bool isResume) {
  while (currentOffset_ < source_.length()) {
    char c = peek();
    if (c == '"') {
      advance(); // Consume quote
      return makeToken(isResume ? TokenType::InterpolatedStringEnd : TokenType::String);
    }
    if (c == '{') {
      advance(); // Consume '{'
      interpolationDepth_++;
      // We return the string part up to and INCLUDING the '{'
      return makeToken(TokenType::InterpolatedStringPart);
    }
    if (c == '\n') line_++;
    advance();
  }

  errors_.report(util::Diagnostic{
      util::DiagnosticsSeverity::Error, {line_, 0, 0, 0}, "Unterminated string.", ""});
  return makeErrorToken("Unterminated string.");
}

} // namespace druk::lexer
