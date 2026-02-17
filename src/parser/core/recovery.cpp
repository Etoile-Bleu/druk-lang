#include "druk/parser/core/parser.hpp"

namespace druk::parser {

void Parser::error(lexer::Token token, std::string_view message) {
  if (panicMode_)
    return;
  panicMode_ = true;

  util::Diagnostic diagnostic;
  diagnostic.severity = util::DiagnosticsSeverity::Error;
  diagnostic.location = {token.line, 0, token.offset, token.length};
  diagnostic.message = std::string(message);
  
  errors_.report(diagnostic);
}

void Parser::synchronize() {
  panicMode_ = false;
  while (!isAtEnd()) {
    if (previous_.type == lexer::TokenType::Semicolon)
      return;

    switch (current_.type) {
    case lexer::TokenType::KwFunction:
    case lexer::TokenType::KwIf:
    case lexer::TokenType::KwLoop:
    case lexer::TokenType::KwReturn:
    case lexer::TokenType::KwPrint:
      return;
    default:;
    }
    advance();
  }
}

} // namespace druk::parser
