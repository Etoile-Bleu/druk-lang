#pragma once

#include "druk/lexer/token.hpp"
#include <string_view>
#include <unordered_map>
#include <array>

namespace druk {

// Forward declarations
class ArenaAllocator;
class StringInterner;
class ErrorReporter;

/**
 * Optimized lexer with fast-path for ASCII characters.
 * Falls back to Unicode handling for Tibetan/Dzongkha script.
 */
class OptimizedLexer {
public:
  OptimizedLexer(std::string_view source, ArenaAllocator &arena,
                 StringInterner &interner, ErrorReporter &errors);

  Token next();

private:
  // Fast-path inline helpers
  inline char peek() const {
    return current_offset_ < source_.length() ? source_[current_offset_] : '\0';
  }

  inline char peek_next() const {
    return current_offset_ + 1 < source_.length() ? source_[current_offset_ + 1] : '\0';
  }

  inline char advance() {
    return source_[current_offset_++];
  }

  inline bool match(char expected) {
    if (peek() != expected)
      return false;
    current_offset_++;
    return true;
  }

  // Inline ASCII checks (much faster than std::isalpha)
  inline bool is_ascii_alpha(unsigned char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
  }

  inline bool is_ascii_digit(unsigned char c) const {
    return c >= '0' && c <= '9';
  }

  inline bool is_ascii_alnum(unsigned char c) const {
    return is_ascii_alpha(c) || is_ascii_digit(c);
  }

  // Token creation
  Token make_token(TokenKind kind);
  Token make_error_token(const char *message);

  // Whitespace skipping with inline fast-path
  void skip_whitespace();
  void skip_whitespace_fast();

  // Scanning functions
  Token scan_ascii_identifier();
  Token scan_ascii_number();
  Token scan_unicode_identifier();
  Token scan_unicode_number();
  Token scan_string();

  // Keyword lookup with perfect hash
  TokenKind check_keyword(std::string_view text);
  TokenKind check_ascii_keyword(std::string_view text);
  
  // Unicode helpers (slow path)
  bool is_tibetan_start(unsigned char c) const { return c == 0xE0; }
  bool is_tibetan_digit_start(unsigned char c) const { return c == 0xE0; }

  std::string_view source_;
  ArenaAllocator &arena_;
  StringInterner &interner_;
  ErrorReporter &errors_;

  size_t current_offset_;
  size_t start_offset_;
  uint32_t line_;

  // Keyword hash map (initialized once)
  static const std::unordered_map<std::string_view, TokenKind> keywords_;
};

} // namespace druk
