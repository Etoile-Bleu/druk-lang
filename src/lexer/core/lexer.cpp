#include "druk/lexer/lexer.hpp"
#include <cctype>

namespace druk::lexer {

Lexer::Lexer(std::string_view source, util::ArenaAllocator &arena,
             StringInterner &interner, util::ErrorHandler &errors)
    : source_(source), arena_(arena), interner_(interner), errors_(errors),
      currentOffset_(0), line_(1) {
  if (source_.length() >= 3 && (unsigned char)source_[0] == 0xEF &&
      (unsigned char)source_[1] == 0xBB && (unsigned char)source_[2] == 0xBF) {
    currentOffset_ = 3;
  }
}

Token Lexer::next() {
  skipWhitespace();
  startOffset_ = currentOffset_;
  if (currentOffset_ >= source_.length()) {
    return makeToken(TokenType::EndOfFile);
  }

  char c = advance();

  if (isTibetanDigitStart(c)) {
    if (currentOffset_ + 1 < source_.length()) {
      unsigned char next1 = static_cast<unsigned char>(source_[currentOffset_]);
      unsigned char next2 = static_cast<unsigned char>(source_[currentOffset_ + 1]);
      if (next1 == 0xBC && (next2 >= 0xA0 && next2 <= 0xA9)) {
        advance();
        advance();
        return scanNumber();
      }
    }
  }

  if (isAlpha(c)) return scanIdentifier();
  if (isDigit(c)) return scanNumber();

  switch (c) {
  case '(': return makeToken(TokenType::LParen);
  case ')': return makeToken(TokenType::RParen);
  case '{': return makeToken(TokenType::LBrace);
  case '}': return makeToken(TokenType::RBrace);
  case '[': return makeToken(TokenType::LBracket);
  case ']': return makeToken(TokenType::RBracket);
  case ',': return makeToken(TokenType::Comma);
  case ':': return makeToken(TokenType::Colon);
  case '.': return makeToken(TokenType::Dot);
  case '-': return makeToken(TokenType::Minus);
  case '+': return makeToken(TokenType::Plus);
  case ';': return makeToken(TokenType::Semicolon);
  case '*': return makeToken(TokenType::Star);
  case '/': return makeToken(TokenType::Slash);
  case '!': return makeToken(match('=') ? TokenType::BangEqual : TokenType::Bang);
  case '&': if (match('&')) return makeToken(TokenType::And); break;
  case '|': if (match('|')) return makeToken(TokenType::Or); break;
  case '=': return makeToken(match('=') ? TokenType::EqualEqual : TokenType::Equal);
  case '<': return makeToken(match('=') ? TokenType::LessEqual : TokenType::Less);
  case '>': return makeToken(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
  case '"': return scanString();
  }

  std::string msg = "Unexpected character: ";
  msg += c;
  return makeErrorToken(msg.c_str());
}

Token Lexer::makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.offset = startOffset_;
  token.length = static_cast<uint32_t>(currentOffset_ - startOffset_);
  token.line = line_;
  token.padding = 0;
  return token;
}

Token Lexer::makeErrorToken([[maybe_unused]] const char *message) {
  Token token;
  token.type = TokenType::Invalid;
  token.offset = startOffset_;
  token.length = static_cast<uint32_t>(currentOffset_ - startOffset_);
  token.line = line_;
  token.padding = 0;
  return token;
}

char Lexer::advance() {
  return source_[currentOffset_++];
}

char Lexer::peek() const {
  if (currentOffset_ >= source_.length()) return '\0';
  return source_[currentOffset_];
}

char Lexer::peekNext() const {
  if (currentOffset_ + 1 >= source_.length()) return '\0';
  return source_[currentOffset_ + 1];
}

bool Lexer::match(char expected) {
  if (peek() != expected) return false;
  currentOffset_++;
  return true;
}

void Lexer::skipWhitespace() {
  while (true) {
    if (currentOffset_ >= source_.length()) return;

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
      if (peekNext() == '/') {
        while (peek() != '\n' && currentOffset_ < source_.length()) advance();
      } else {
        return;
      }
      break;
    default:
      if (static_cast<unsigned char>(c) == 0xE0) {
        if (currentOffset_ + 2 < source_.length()) {
          unsigned char n1 = static_cast<unsigned char>(source_[currentOffset_ + 1]);
          unsigned char n2 = static_cast<unsigned char>(source_[currentOffset_ + 2]);
          if (n1 == 0xBC && n2 == 0x8B) {
            advance(); advance(); advance();
            break;
          }
        }
      }
      return;
    }
  }
}

} // namespace druk::lexer
