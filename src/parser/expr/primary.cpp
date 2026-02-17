#include <cstdlib>
#include <string>
#include <cctype>

#include "druk/parser/core/parser.hpp"

namespace
{
// Convert Tibetan numerals to ASCII
std::string tibetanToAsciiNumber(std::string_view text)
{
    std::string result;
    result.reserve(text.size());
    
    for (size_t i = 0; i < text.size();)
    {
        // Tibetan digits are ༠-༩ (U+0F20 to U+0F29)
        // In UTF-8: E0 BC A0 to E0 BC A9
        if (i + 2 < text.size() && 
            static_cast<unsigned char>(text[i]) == 0xE0 &&
            static_cast<unsigned char>(text[i+1]) == 0xBC &&
            static_cast<unsigned char>(text[i+2]) >= 0xA0 &&
            static_cast<unsigned char>(text[i+2]) <= 0xA9)
        {
            // Convert: ༠=0xA0→'0', ༡=0xA1→'1', etc.
            result += '0' + (text[i+2] - 0xA0);
            i += 3;
        }
        else if (std::isdigit(static_cast<unsigned char>(text[i])))
        {
            result += text[i];
            ++i;
        }
        else
        {
            ++i;  // Skip unknown bytes
        }
    }
    
    return result;
}
}  // anonymous namespace


namespace druk::parser
{

ast::Expr* Parser::parseExpression()
{
    return parseAssignment();
}

ast::Expr* Parser::parsePrimary()
{
    if (match(lexer::TokenType::KwFalse))
    {
        auto* expr         = arena_.make<ast::LiteralExpr>();
        expr->kind         = ast::NodeKind::Literal;
        expr->token        = previous();
        expr->literalValue = codegen::Value(false);
        return expr;
    }
    if (match(lexer::TokenType::KwBoolean))
    {
        auto* expr         = arena_.make<ast::LiteralExpr>();
        expr->kind         = ast::NodeKind::Literal;
        expr->token        = previous();
        expr->literalValue = codegen::Value(true);
        return expr;
    }

    if (match(lexer::TokenType::Number))
    {
        auto* expr         = arena_.make<ast::LiteralExpr>();
        expr->kind         = ast::NodeKind::Literal;
        expr->token        = previous();
        std::string_view text = expr->token.text(lexer_.source());
        std::string asciiNum = tibetanToAsciiNumber(text);
        expr->literalValue = codegen::Value(static_cast<int64_t>(std::atoll(asciiNum.c_str())));
        return expr;
    }

    if (match(lexer::TokenType::String))
    {
        auto* expr            = arena_.make<ast::LiteralExpr>();
        expr->kind            = ast::NodeKind::Literal;
        expr->token           = previous();
        std::string_view text = expr->token.text(lexer_.source());
        // Strip quotes and intern the string for stable storage
        std::string_view value = interner_.intern(text.substr(1, text.length() - 2));
        expr->literalValue     = codegen::Value(value);
        return expr;
    }

    if (match(lexer::TokenType::Identifier))
    {
        auto* expr = arena_.make<ast::VariableExpr>();
        expr->kind = ast::NodeKind::VariableExpr;
        expr->name = previous();
        return expr;
    }

    if (match(lexer::TokenType::LParen))
    {
        ast::Expr* expr = parseExpression();
        consume(lexer::TokenType::RParen, "Expect ')' after expression.");
        return expr;
    }

    if (check(lexer::TokenType::LBracket))
        return parseArrayLiteral();
    if (check(lexer::TokenType::LBrace))
        return parseStructLiteral();

    error(peek(), "Expect expression.");
    return nullptr;
}

}  // namespace druk::parser
