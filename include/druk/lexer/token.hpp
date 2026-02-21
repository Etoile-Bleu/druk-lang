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
    InterpolatedStringPart,
    InterpolatedStringEnd,

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
    Question,
    KwNil,
    Underscore,
    Pipe,

    Count
};

struct Token
{
    TokenType type    = TokenType::EndOfFile;
    uint16_t  padding = 0;
    uint32_t  offset  = 0;
    uint32_t  length  = 0;
    uint32_t  line    = 1;
    uint32_t  column  = 0;

    [[nodiscard]] std::string_view text(std::string_view source) const
    {
        if (offset + length > source.length())
            return "";
        return source.substr(offset, length);
    }
};

static_assert(sizeof(Token) == 20, "Token must be 20 bytes");

}  // namespace druk::lexer
