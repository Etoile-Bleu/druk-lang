#include <cctype>
#include <cstdlib>
#include <string>

#include "druk/gc/gc_heap.h"
#include "druk/gc/types/gc_string.h"
#include "druk/parser/core/parser.hpp"

namespace
{
std::string tibetanToAsciiNumber(std::string_view text)
{
    std::string result;
    result.reserve(text.size());

    for (size_t i = 0; i < text.size();)
    {
        if (i + 2 < text.size() && static_cast<unsigned char>(text[i]) == 0xE0 &&
            static_cast<unsigned char>(text[i + 1]) == 0xBC &&
            static_cast<unsigned char>(text[i + 2]) >= 0xA0 &&
            static_cast<unsigned char>(text[i + 2]) <= 0xA9)
        {
            result += '0' + (text[i + 2] - 0xA0);
            i += 3;
        }
        else if (std::isdigit(static_cast<unsigned char>(text[i])))
        {
            result += text[i];
            ++i;
        }
        else
        {
            ++i;
        }
    }

    return result;
}
}  // namespace

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
    if (match(lexer::TokenType::KwTrue))
    {
        auto* expr         = arena_.make<ast::LiteralExpr>();
        expr->kind         = ast::NodeKind::Literal;
        expr->token        = previous();
        expr->literalValue = codegen::Value(true);
        return expr;
    }

    if (match(lexer::TokenType::Number))
    {
        auto* expr                = arena_.make<ast::LiteralExpr>();
        expr->kind                = ast::NodeKind::Literal;
        expr->token               = previous();
        std::string_view text     = expr->token.text(lexer_.source());
        std::string      asciiNum = tibetanToAsciiNumber(text);
        expr->literalValue = codegen::Value(static_cast<int64_t>(std::atoll(asciiNum.c_str())));
        return expr;
    }

    if (match(lexer::TokenType::String))
    {
        auto* expr            = arena_.make<ast::LiteralExpr>();
        expr->kind            = ast::NodeKind::Literal;
        expr->token           = previous();
        std::string_view text = expr->token.text(lexer_.source());
        auto*            gs =
            gc::GcHeap::get().alloc<gc::GcString>(std::string(text.substr(1, text.length() - 2)));
        expr->literalValue = codegen::Value(gs);
        return expr;
    }

    if (match(lexer::TokenType::Identifier))
    {
        lexer::Token name = previous();
        if (match(lexer::TokenType::Arrow))
        {
            // Single parameter lambda
            return parseLambdaFromIdentifier(name);
        }
        auto* expr = arena_.make<ast::VariableExpr>();
        expr->kind = ast::NodeKind::VariableExpr;
        expr->name = name;
        return expr;
    }

    if (match(lexer::TokenType::LParen))
    {
        bool isLambda = false;
        if (check(lexer::TokenType::RParen) && peekNext().type == lexer::TokenType::Arrow)
            isLambda = true;
        else if (check(lexer::TokenType::Identifier) &&
                 (peekNext().type == lexer::TokenType::Comma ||
                  peekNext().type == lexer::TokenType::RParen))
        {
            isLambda = true;
        }

        if (isLambda)
            return parseLambda();

        ast::Expr* expr = parseExpression();
        consume(lexer::TokenType::RParen, "Expect ')' after expression.");
        auto* group       = arena_.make<ast::GroupingExpr>();
        group->kind       = ast::NodeKind::Grouping;
        group->expression = expr;
        return group;
    }

    if (check(lexer::TokenType::LBracket))
        return parseArrayLiteral();
    if (check(lexer::TokenType::LBrace))
        return parseStructLiteral();

    error(peek(), "Expect expression.");
    return nullptr;
}

}  // namespace druk::parser
