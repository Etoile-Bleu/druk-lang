#pragma once

#include <optional>
#include <string_view>

#include "druk/lexer/token.hpp"
#include "druk/lexer/unicode.hpp"
#include "druk/util/arena_allocator.hpp"
#include "druk/util/error_handler.hpp"


namespace druk::lexer
{

/**
 * @brief The Lexer class performs lexical analysis on Druk source code.
 */
class Lexer
{
   public:
    /**
     * @brief Constructs a Lexer with a source string and required dependencies.
     */
    Lexer(std::string_view source, util::ArenaAllocator& arena, StringInterner& interner,
          util::ErrorHandler& errors);

    /**
     * @brief Scans the next token from the source.
     * @return The scanned Token.
     */
    Token next();

    /**
     * @brief Returns the original source code.
     */
    [[nodiscard]] std::string_view source() const
    {
        return source_;
    }

   private:
    // Helpers
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);
    void skipWhitespace();

    // Scanners
    Token scanIdentifier();
    Token scanNumber();
    Token scanString(bool isResume = false);

    Token makeToken(TokenType type);
    Token makeErrorToken(const char* message);

    // Keyword lookup
    TokenType checkKeyword(std::string_view text);

    bool isAlpha(char c);
    bool isDigit(char c);
    bool isTibetanDigitStart(char c);

    std::string_view      source_;
    util::ArenaAllocator& arena_;
    StringInterner&       interner_;
    util::ErrorHandler&   errors_;

    uint32_t startOffset_   = 0;  // Start of current token
    uint32_t currentOffset_ = 0;  // Current scanning position
    uint32_t line_          = 1;
    uint32_t column_        = 1;

    uint32_t interpolationDepth_ = 0;
};

}  // namespace druk::lexer
