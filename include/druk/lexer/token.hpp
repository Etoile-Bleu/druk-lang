#pragma once

#include <cstdint>
#include <string_view>

namespace druk::lexer {

/**
 * @brief Categorizes the different types of tokens in the Druk language.
 */
enum class TokenType : uint16_t {
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
  KwBoolean,  // བདེན
  KwIf,       // གལ་ཏེ
  KwElse,     // ཡང་ན
  KwLoop,     // རིམ་པ
  KwReturn,   // ལོག
  KwPrint,    // འབྲི
  KwTrue,     // བདེན
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
  LBracket,  // [
  RBracket,  // ]
  Comma,     // ,
  Semicolon, // ;
  Colon,     // :
  Dot,       // .

  Count
};

/**
 * @brief Represents a single lexical token.
 */
struct Token {
  TokenType type;
  uint16_t padding; // Padding to align
  uint32_t offset;
  uint32_t length;
  uint32_t line;

  /**
   * @brief Returns the text representation of the token from the source.
   * @param source The source code string.
   * @return std::string_view containing the token text.
   */
  [[nodiscard]] std::string_view text(std::string_view source) const {
    return source.substr(offset, length);
  }
};

static_assert(sizeof(Token) == 16, "Token must be 16 bytes");

} // namespace druk::lexer
