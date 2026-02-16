#include "druk/parser/core/parser.hpp"

namespace druk {

Parser::Parser(std::string_view source, ArenaAllocator &arena,
               StringInterner &interner, ErrorReporter &errors)
    : lexer_(source, arena, interner, errors), arena_(arena),
      interner_(interner), errors_(errors), panic_mode_(false) {
  advance(); // Seed current_
}

std::vector<Stmt *> Parser::parse() {
  std::vector<Stmt *> statements;
  while (!is_at_end()) {
    Stmt *stmt = parse_declaration();
    if (stmt) {
      statements.push_back(stmt);
    } else {
      synchronize();
    }
  }
  return statements;
}

Token Parser::advance() {
  previous_ = current_;
  while (true) {
    current_ = lexer_.next();
    if (current_.kind != TokenKind::Invalid)
      break;
  }
  return previous_;
}

bool Parser::check(TokenKind kind) const {
  if (is_at_end())
    return false;
  return current_.kind == kind;
}

Token Parser::consume(TokenKind kind, std::string_view message) {
  if (check(kind))
    return advance();
  error(current_, message);

  // Still advance to avoid getting stuck in infinite loop
  if (current_.kind != TokenKind::EndOfFile) {
    advance();
  }

  return current_;
}

bool Parser::match(TokenKind kind) {
  if (check(kind)) {
    advance();
    return true;
  }
  return false;
}

} // namespace druk
