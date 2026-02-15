#include "druk/lexer/lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace druk {

Lexer::Lexer(std::string_view source, ArenaAllocator &arena,
             StringInterner &interner, ErrorReporter &errors)
    : source_(source), arena_(arena), interner_(interner), errors_(errors) {
  // Skip initial BOM if present?
  // Usually UTF-8 files might have EF BB BF.
  if (source_.size() >= 3 && (unsigned char)source_[0] == 0xEF &&
      (unsigned char)source_[1] == 0xBB && (unsigned char)source_[2] == 0xBF) {
    current_offset_ += 3;
  }
}

static bool is_alpha(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

static bool is_digit(char c) {
  return std::isdigit(static_cast<unsigned char>(c));
}

// Tibetan digits 0-9: ༠ ༡ ༢ ༣ ༤ ༥ ༦ ༧ ༨ ༩
// Unicode: U+0F20 to U+0F29
// UTF-8: E0 BC A0 to E0 BC A9
static bool is_tibetan_digit_start(char c) { return (unsigned char)c == 0xE0; }

// Helper to check if we are at a Tibetan digit and consume it
// This is a bit tricky inside a char-by-char loop.
// We'll handle it in scan_number or the main loop.

Token Lexer::next() {
  skip_whitespace();
  start_offset_ = current_offset_;

  if (current_offset_ >= source_.length()) {
    return make_token(TokenKind::EndOfFile);
  }

  char c = advance();

  if (is_alpha(c))
    return scan_identifier();
  if (is_digit(c))
    return scan_number();

  // Handle Tibetan characters (start with high bit set)
  // Most Tibetan letters start with 0xE0 0xBD ... or similar.
  // Tibetan digits start with 0xE0 0xBC ...
  if ((unsigned char)c >= 0x80) {
    // Assume it's an identifier or number
    // We need to look at specific bytes for digits if we want to separate logic
    // strict But scan_identifier handles UTF-8 logic generically. Let's peek to
    // see if it's a digit 0xE0 0xBC 0xA0-0xA9
    if (is_tibetan_digit_start(c)) {
      if (current_offset_ + 1 < source_.length()) {
        unsigned char next1 = (unsigned char)source_[current_offset_];
        unsigned char next2 = (unsigned char)source_[current_offset_ + 1];
        if (next1 == 0xBC && (next2 >= 0xA0 && next2 <= 0xA9)) {
          // It is a Tibetan digit!
          // We consumed 0xE0 (c). Now consume 0xBC and the digit byte.
          advance(); // 0xBC
          advance(); // 0xA0-0xA9
          return scan_number();
        }
      }
    }
    return scan_identifier();
  }

  switch (c) {
  case '(':
    return make_token(TokenKind::LParen);
  case ')':
    return make_token(TokenKind::RParen);
  case '{':
    return make_token(TokenKind::LBrace);
  case '}':
    return make_token(TokenKind::RBrace);
  case ';':
    return make_token(TokenKind::Semicolon);
  case ',':
    return make_token(TokenKind::Comma);
  case '.':
    return make_token(TokenKind::Invalid); // Dot?
  case '-':
    return make_token(TokenKind::Minus);
  case '+':
    return make_token(TokenKind::Plus);
  case '/':
    return make_token(TokenKind::Slash);
  case '*':
    return make_token(TokenKind::Star);

  case '!':
    return make_token(match('=') ? TokenKind::BangEqual : TokenKind::Bang);
  case '=':
    return make_token(match('=') ? TokenKind::EqualEqual : TokenKind::Equal);
  case '<':
    return make_token(match('=') ? TokenKind::LessEqual : TokenKind::Less);
  case '>':
    return make_token(match('=') ? TokenKind::GreaterEqual
                                 : TokenKind::Greater);

  case '"':
    return scan_string();

  case '&':
    if (match('&'))
      return make_token(TokenKind::And);
    break;
  case '|':
    if (match('|'))
      return make_token(TokenKind::Or);
    break;
  }

  // Report error
  std::string msg = "Unexpected character: ";
  msg += c;
  return make_error_token(msg.c_str()); // c_str copy issue?
}

void Lexer::skip_whitespace() {
  while (true) {
    if (current_offset_ >= source_.length())
      return;
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      line_++;
      advance();
      break;
    case '/':
      if (peek_next() == '/') {
        while (peek() != '\n' && current_offset_ < source_.length())
          advance();
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

Token Lexer::scan_string() {
  while (peek() != '"' && current_offset_ < source_.length()) {
    if (peek() == '\n')
      line_++;
    advance();
  }

  if (current_offset_ >= source_.length()) {
    return make_error_token("Unterminated string.");
  }

  advance(); // Closing quote

  // +1 and -1 to strip quotes? Or keep them?
  // Token usually keeps raw text range.
  Token token = make_token(TokenKind::String);

  // Intern the content (without quotes)
  // This is optional if Token just points to source.
  // But for AST we likely want the identifier/string content.
  // The Token struct doesn't have a field for the interned value, just
  // offset/len. The parser will intern it if needed.

  return token;
}

Token Lexer::scan_number() {
  while (true) {
    char c = peek();
    if (is_digit(c)) {
      advance();
    } else if ((unsigned char)c == 0xE0) {
      // Check for Tibetan digit
      if (current_offset_ + 2 < source_.length()) {
        unsigned char next1 = (unsigned char)source_[current_offset_ + 1];
        unsigned char next2 = (unsigned char)source_[current_offset_ + 2];
        if (next1 == 0xBC && (next2 >= 0xA0 && next2 <= 0xA9)) {
          advance();
          advance();
          advance();
          continue;
        }
      }
      break;
    } else {
      break;
    }
  }
  return make_token(TokenKind::Number);
}

Token Lexer::scan_identifier() {
  while (true) {
    char c = peek();
    if (is_alpha(c) || is_digit(c)) {
      advance();
    } else if ((unsigned char)c >= 0x80) {
      // Assume unicode char part of identifier
      // We should strictly use unicode properties (is_alpha etc)
      // For now, accept all high-bit bytes as identifier chars
      advance();
    } else {
      break;
    }
  }

  std::string_view text =
      source_.substr(start_offset_, current_offset_ - start_offset_);
  TokenKind kind = check_keyword(text);
  return make_token(kind);
}

TokenKind Lexer::check_keyword(std::string_view text) {
  static const std::unordered_map<std::string_view, TokenKind> keywords = {
      {"ལས་ཀ", TokenKind::KwFunction},
      {"གྲངས་ཀ", TokenKind::KwNumber},
      {"ཡིག་རྟགས", TokenKind::KwString},
      {"བདེན", TokenKind::KwBoolean}, // Also True? Or Contextual?
      {"རྫུན", TokenKind::KwFalse},
      {"གལ་ཏེ", TokenKind::KwIf},
      {"ཡང་ན", TokenKind::KwElse},
      {"རིམ་པ", TokenKind::KwLoop},
      {"ལོག", TokenKind::KwReturn},
      {"འབྲི", TokenKind::KwPrint}};

  if (text == "བདེན") {
    // This is ambiguous in our spec: བདེན is type 'bool' AND value 'true'?
    // Usually we want distinct tokens or handle in parser.
    // Let's assume KwTrue for now if it's a value, but if used as type...
    // Actually spec says: བདེན (den) -> boolean AND བདེན -> true.
    // We can return KwBoolean and Parser accepts KwBoolean as a literal too.
    return TokenKind::KwBoolean;
  }

  auto it = keywords.find(text);
  if (it != keywords.end())
    return it->second;

  return TokenKind::Identifier;
}

Token Lexer::make_token(TokenKind kind) {
  Token token;
  token.kind = kind;
  token.offset = start_offset_;
  token.length = current_offset_ - start_offset_;
  token.line = line_;
  token._padding = 0;
  return token;
}

Token Lexer::make_error_token(std::string_view message) {
  Token token = make_token(TokenKind::Invalid);
  errors_.report(Error{ErrorLevel::Error,
                       {line_, 0, start_offset_, token.length},
                       std::string(message),
                       ""});
  return token;
}

char Lexer::peek() const {
  if (current_offset_ >= source_.length())
    return '\0';
  return source_[current_offset_];
}

char Lexer::peek_next() const {
  if (current_offset_ + 1 >= source_.length())
    return '\0';
  return source_[current_offset_ + 1];
}

char Lexer::advance() {
  if (current_offset_ >= source_.length())
    return '\0';
  return source_[current_offset_++];
}

bool Lexer::match(char expected) {
  if (current_offset_ >= source_.length())
    return false;
  if (source_[current_offset_] != expected)
    return false;
  current_offset_++;
  return true;
}

} // namespace druk
