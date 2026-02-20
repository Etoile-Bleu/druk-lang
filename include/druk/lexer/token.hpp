#pragma once

#include <cstdint>
#include <string_view>

namespace druk::lexer
{

enum class TokenType : uint16_t
{
    EndOfFile = 0,
    Invalid,

    // Identifiers & Literals
    Identifier,
    Number,
    String,

    // Keywords
    KwFunction,  // ལས་འགན་
    KwNumber,    // གྲངས་
    KwString,    // ཡིག་འབྲུ་
    KwBoolean,   // བདེན་རྫུན་
    KwVoid,      // སྟོང་པ
    KwIf,        // གལ་སྲིད་
    KwElse,      // མེད་ན་
    KwLoop,      // ཡང་བསྐྱར་  (legacy alias)
    KwWhile,     // ཡང་བསྐྱར་
    KwFor,       // རེ་རེར་
    KwReturn,
    KwPrint,
    KwTrue,
    KwFalse,
    KwMatch,

    Plus,
    Minus,
    Star,
    Slash,
    Equal,
    EqualEqual,
    Bang,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    And,
    Or,
    Arrow,

    LParen,
    RParen,
    LBrace,
    RBrace,
    LBracket,
    RBracket,
    Comma,
    Semicolon,
    Colon,
    Dot,
    Underscore,

    Count
};

struct Token
{
    TokenType type;
    uint16_t  padding;  // Padding to align
    uint32_t  offset;
    uint32_t  length;
    uint32_t  line;

    [[nodiscard]] std::string_view text(std::string_view source) const
    {
        return source.substr(offset, length);
    }
};

static_assert(sizeof(Token) == 16, "Token must be 16 bytes");

}  // namespace druk::lexer
