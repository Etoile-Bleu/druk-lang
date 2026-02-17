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
        {"ལས་ཀ་", TokenType::KwFunction}, {"གྲངས་ཀ་", TokenType::KwNumber},
        {"ཡིག་རྟགས་", TokenType::KwString}, {"བདེན་", TokenType::KwBoolean},
        {"རྫུན་", TokenType::KwFalse},      {"གལ་ཏེ་", TokenType::KwIf},
        {"ཡང་ན་", TokenType::KwElse},     {"རིམ་པ་", TokenType::KwLoop},
        {"ལོག་", TokenType::KwReturn},     {"འབྲི་", TokenType::KwPrint}};

    auto it = keywords.find(text);
    if (it != keywords.end())
        return it->second;

    return TokenType::Identifier;
}

}  // namespace druk::lexer
