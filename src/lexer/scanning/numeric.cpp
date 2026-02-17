#include "druk/lexer/lexer.hpp"
#include <cctype>

namespace druk::lexer {

Token Lexer::scanNumber() {
  while (true) {
    char c = peek();
    if (isDigit(c)) {
      advance();
    } else if (isTibetanDigitStart(c)) {
      if (currentOffset_ + 2 < source_.length()) {
        unsigned char next1 = static_cast<unsigned char>(source_[currentOffset_ + 1]);
        unsigned char next2 = static_cast<unsigned char>(source_[currentOffset_ + 2]);
        if (next1 == 0xBC && (next2 >= 0xA0 && next2 <= 0xA9)) {
          advance(); advance(); advance();
          continue;
        }
      }
      break;
    } else {
      break;
    }
  }
  return makeToken(TokenType::Number);
}

bool Lexer::isDigit(char c) {
  return std::isdigit(static_cast<unsigned char>(c));
}

bool Lexer::isTibetanDigitStart(char c) {
  return static_cast<unsigned char>(c) == 0xE0;
}

} // namespace druk::lexer
