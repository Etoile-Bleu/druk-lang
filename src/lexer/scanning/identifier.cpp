#include <cctype>
#include <unordered_map>

#include "druk/lexer/lexer.hpp"

namespace druk::lexer
{

Token Lexer::scanIdentifier()
{
    while (true)
    {
        char c = peek();
        if (isAlpha(c) || isDigit(c))
        {
            advance();
        }
        else
        {
            break;
        }
    }

    std::string_view text = source_.substr(startOffset_, currentOffset_ - startOffset_);
    return makeToken(checkKeyword(text));
}

bool Lexer::isAlpha(char c)
{
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_' ||
           static_cast<unsigned char>(c) >= 0x80;
}

TokenType Lexer::checkKeyword(std::string_view text)
{
    static const std::unordered_map<std::string_view, TokenType> keywords = {
        {"ལས་འགན་", TokenType::KwFunction}, {"གྲངས་", TokenType::KwNumber},
        {"ཡིག་འབྲུ་", TokenType::KwString},    {"བདེན་རྫུན་", TokenType::KwBoolean},
        {"བདེན་པ་", TokenType::KwTrue},      {"རྫུན་མ་", TokenType::KwFalse},
        {"གལ་སྲིད་", TokenType::KwIf},        {"མེད་ན་", TokenType::KwElse},
        {"ཡང་བསྐྱར་", TokenType::KwWhile},    {"རེ་རེར་", TokenType::KwFor},
        {"རིམ་པ་", TokenType::KwLoop},       {"སླར་ལོག་", TokenType::KwReturn},
        {"བཀོད་", TokenType::KwPrint},       {"འགྲིག་པ་", TokenType::KwMatch},
        {"སྟོང་པ", TokenType::KwVoid},        {"ཅི་མེད", TokenType::KwNil}};

    if (auto it = keywords.find(text); it != keywords.end())
    {
        return it->second;
    }

    return TokenType::Identifier;
}

}  // namespace druk::lexer
