#include "druk/lexer/lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace druk {

Lexer::Lexer(std::string_view source, ArenaAllocator &arena,
             StringInterner &interner, ErrorReporter &errors)
    : source_(source), arena_(arena), interner_(interner), errors_(errors),
      current_offset_(0), line_(1) {
  // Skip UTF-8 BOM if present
  if (source_.length() >= 3 && (unsigned char)source_[0] == 0xEF &&
      (unsigned char)source_[1] == 0xBB && (unsigned char)source_[2] == 0xBF) {
    current_offset_ = 3;
  }
}

Token Lexer::next() {
  skip_whitespace();
  start_offset_ = current_offset_;
  if (current_offset_ >= source_.length()) {
    return make_token(TokenKind::EndOfFile);
  }

  char c = advance();

  if (is_tibetan_digit_start(c)) {
    if (current_offset_ + 1 < source_.length()) {
      unsigned char next1 = (unsigned char)source_[current_offset_];
      unsigned char next2 = (unsigned char)source_[current_offset_ + 1];
      if (next1 == 0xBC && (next2 >= 0xA0 && next2 <= 0xA9)) {
        advance(); // next1
        advance(); // next2
        return scan_number();
      }
    }
  }

  if (is_alpha(c))
    return scan_identifier();
  if (is_digit(c))
    return scan_number();

  switch (c) {
  case '(':
    return make_token(TokenKind::LParen);
  case ')':
    return make_token(TokenKind::RParen);
  case '{':
    return make_token(TokenKind::LBrace);
  case '}':
    return make_token(TokenKind::RBrace);
  case '[':
    return make_token(TokenKind::LBracket);
  case ']':
    return make_token(TokenKind::RBracket);
  case ',':
    return make_token(TokenKind::Comma);
  case ':':
    return make_token(TokenKind::Colon);
  case '.':
    return make_token(TokenKind::Dot);
  case '-':
    return make_token(TokenKind::Minus);
  case '+':
    return make_token(TokenKind::Plus);
  case ';':
    return make_token(TokenKind::Semicolon);
  case '*':
    return make_token(TokenKind::Star);
  case '/':
    return make_token(TokenKind::Slash);
  case '!':
    return make_token(match('=') ? TokenKind::BangEqual : TokenKind::Bang);
  case '&':
    if (match('&'))
      return make_token(TokenKind::And);
    break;
  case '|':
    if (match('|'))
      return make_token(TokenKind::Or);
    break;
  case '=':
    return make_token(match('=') ? TokenKind::EqualEqual : TokenKind::Equal);
  case '<':
    return make_token(match('=') ? TokenKind::LessEqual : TokenKind::Less);
  case '>':
    return make_token(match('=') ? TokenKind::GreaterEqual
                                 : TokenKind::Greater);
  case '"':
    return scan_string();
  }

  // Report error
  std::string msg = "Unexpected character: ";
  msg += c;
  return make_error_token(msg.c_str());
}

Token Lexer::make_token(TokenKind kind) {
  Token token;
  token.kind = kind;
  token.offset = start_offset_;
  token.length = static_cast<uint32_t>(current_offset_ - start_offset_);
  token.line = line_;
  return token;
}

Token Lexer::make_error_token([[maybe_unused]] const char *message) {
  Token token;
  token.kind = TokenKind::Invalid;
  token.offset = start_offset_;
  token.length = static_cast<uint32_t>(current_offset_ - start_offset_);
  token.line = line_;
  return token;
}

char Lexer::advance() {
  char c = source_[current_offset_++];
  return c;
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

bool Lexer::match(char expected) {
  if (peek() != expected)
    return false;
  current_offset_++;
  return true;
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
        // Comment
        while (peek() != '\n' && current_offset_ < source_.length())
          advance();
      } else {
        return;
      }
      break;
    default:
      if ((unsigned char)c == 0xE0) {
        if (current_offset_ + 2 < source_.length()) {
          unsigned char n1 = (unsigned char)source_[current_offset_ + 1];
          unsigned char n2 = (unsigned char)source_[current_offset_ + 2];
          if (n1 == 0xBC && n2 == 0x8B) {
            advance();
            advance();
            advance();
            break;
          }
        }
      }
      return;
    }
  }
}

Token Lexer::scan_identifier() {
  while (true) {
    char c = peek();
    if (is_alpha(c) || is_digit(c)) {
      advance();
    } else {
      break;
    }
  }

  std::string_view text =
      source_.substr(start_offset_, current_offset_ - start_offset_);
  return make_token(check_keyword(text));
}

Token Lexer::scan_number() {
  while (true) {
    char c = peek();
    if (is_digit(c)) {
      advance();
    } else if ((unsigned char)c == 0xE0) {
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

Token Lexer::scan_string() {
  while (peek() != '"' && current_offset_ < source_.length()) {
    if (peek() == '\n')
      line_++;
    advance();
  }

  if (current_offset_ >= source_.length()) {
    errors_.report(
        Error{ErrorLevel::Error, {line_, 0, 0, 0}, "Unterminated string.", ""});
    return make_error_token("Unterminated string.");
  }

  advance();
  return make_token(TokenKind::String);
}

bool Lexer::is_alpha(char c) {
  return std::isalpha((unsigned char)c) || c == '_' || (unsigned char)c >= 0x80;
}

bool Lexer::is_digit(char c) { return std::isdigit((unsigned char)c); }

bool Lexer::is_tibetan_digit_start(char c) { return (unsigned char)c == 0xE0; }

TokenKind Lexer::check_keyword(std::string_view text) {
  static const std::unordered_map<std::string_view, TokenKind> keywords = {
      {"ལས་ཀ་", TokenKind::KwFunction}, {"གྲངས་ཀ་", TokenKind::KwNumber},
      {"ཡིག་རྟགས་", TokenKind::KwString}, {"བདེན་", TokenKind::KwBoolean},
      {"རྫུན་", TokenKind::KwFalse},      {"གལ་ཏེ་", TokenKind::KwIf},
      {"ཡང་ན་", TokenKind::KwElse},     {"རིམ་པ་", TokenKind::KwLoop},
      {"ལོག་", TokenKind::KwReturn},     {"འབྲི་", TokenKind::KwPrint}};

  auto it = keywords.find(text);
  if (it != keywords.end())
    return it->second;

  return TokenKind::Identifier;
}

} // namespace druk
