#pragma once

#include "druk/common/allocator.hpp"
#include "druk/common/error.hpp"
#include "druk/lexer/token.hpp"
#include "druk/lexer/unicode.hpp"
#include <optional>
#include <string_view>


namespace druk {

class Lexer {
public:
  Lexer(std::string_view source, ArenaAllocator &arena,
        StringInterner &interner, ErrorReporter &errors);

  // Get the next token. Returns TokenKind::EndOfFile when done.
  Token next();

private:
  // Helpers
  char peek() const;
  char peek_next() const;
  char advance();
  bool match(char expected);
  void skip_whitespace();

  // Scanners
  Token scan_identifier();
  Token scan_number();
  Token scan_string();

  Token make_token(TokenKind kind);
  Token make_error_token(std::string_view message);

  // Keyword lookup
  TokenKind check_keyword(std::string_view text);

  std::string_view source_;
  ArenaAllocator &arena_;
  StringInterner &interner_;
  ErrorReporter &errors_;

  uint32_t start_offset_ = 0;   // Start of current token
  uint32_t current_offset_ = 0; // Current scanning position
  uint32_t line_ = 1;
};

} // namespace druk
