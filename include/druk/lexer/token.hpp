#pragma once

#include <cstdint>
#include <string_view>

namespace druk {

enum class TokenKind : uint16_t {
  EndOfFile = 0,
  Invalid,

  // Identifiers & Literals
  Identifier,
  Number,
  String,

  // Keywords
  KwFunction, // ལས་ཀ
  KwNumber,   // གྲངས་ཀ
  KwString,   // ཡིག་རྟགས
  KwBoolean,  // བདེན (type) / བདེན (true) handle contextually or separate?
  KwIf,       // གལ་ཏེ
  KwElse,     // ཡང་ན
  KwLoop,     // རིམ་པ
  KwReturn,   // ལོག
  KwPrint,    // འབྲི
  KwTrue,     // བདེན (val)
  KwFalse,    // རྫུན

  // Operators
  Plus,         // +
  Minus,        // -
  Star,         // *
  Slash,        // /
  Equal,        // =
  EqualEqual,   // ==
  Bang,         // !
  BangEqual,    // !=
  Less,         // <
  LessEqual,    // <=
  Greater,      // >
  GreaterEqual, // >=
  And,          // &&
  Or,           // ||

  // Punctuation
  LParen,    // (
  RParen,    // )
  LBrace,    // {
  RBrace,    // }
  Comma,     // ,
  Semicolon, // ;

  Count
};

struct Token {
  TokenKind kind;
  uint16_t _padding; // Padding to align
  uint32_t offset;
  uint32_t length;
  uint32_t line;

  // 2 (kind) + 2 (pad) + 4 (off) + 4 (len) + 4 (line) = 16 bytes
  // Optimized for size.

  [[nodiscard]] std::string_view text(std::string_view source) const {
    return source.substr(offset, length);
  }
};

static_assert(sizeof(Token) == 16, "Token must be 16 bytes");

} // namespace druk
