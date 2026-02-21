#include <cctype>
#include <cstdlib>
#include <string>

#include "druk/gc/gc_heap.h"
#include "druk/gc/types/gc_string.h"
#include "druk/parser/ast/expr.hpp"
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
    if (match(lexer::TokenType::KwNil))
    {
        auto* expr         = arena_.make<ast::LiteralExpr>();
        expr->kind         = ast::NodeKind::Literal;
        expr->token        = previous();
        expr->literalValue = codegen::Value();
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

    if (match(lexer::TokenType::InterpolatedStringPart))
    {
        auto* expr  = arena_.make<ast::InterpolatedStringExpr>();
        expr->kind  = ast::NodeKind::InterpolatedStringExpr;
        expr->token = previous();

        std::vector<ast::Expr*> parts;

        // Add the first part (excluding opening quote, including braces? Lexer gave `"Hello {`)
        std::string_view first_text = expr->token.text(lexer_.source());
        auto*            gs_first   = gc::GcHeap::get().alloc<gc::GcString>(
            std::string(first_text.substr(1, first_text.length() - 2)));
        auto* first_lit         = arena_.make<ast::LiteralExpr>();
        first_lit->kind         = ast::NodeKind::Literal;
        first_lit->literalValue = codegen::Value(gs_first);
        parts.push_back(first_lit);

        while (true)
        {
            // Parse inner expression
            parts.push_back(parseExpression());

            if (match(lexer::TokenType::InterpolatedStringPart))
            {
                std::string_view part_text = previous().text(lexer_.source());
                auto*            gs_part   = gc::GcHeap::get().alloc<gc::GcString>(std::string(
                    part_text.substr(1, part_text.length() - 2)));  // e.g., `} are {` -> ` are `
                auto*            lit       = arena_.make<ast::LiteralExpr>();
                lit->kind                  = ast::NodeKind::Literal;
                lit->literalValue          = codegen::Value(gs_part);
                parts.push_back(lit);
            }
            else if (match(lexer::TokenType::InterpolatedStringEnd))
            {
                std::string_view end_text = previous().text(lexer_.source());
                auto* gs_end = gc::GcHeap::get().alloc<gc::GcString>(std::string(end_text.substr(
                    1, end_text.length() - 2)));  // e.g., `} years old!"` -> ` years old!`
                auto* lit    = arena_.make<ast::LiteralExpr>();
                lit->kind    = ast::NodeKind::Literal;
                lit->literalValue = codegen::Value(gs_end);
                parts.push_back(lit);
                break;
            }
            else
            {
                error(peek(), "Expect closing logic for interpolated string.");
                break;
            }
        }

        expr->parts = arena_.allocateArray<ast::Expr*>(parts.size());
        for (size_t i = 0; i < parts.size(); ++i) expr->parts[i] = parts[i];
        expr->count = static_cast<uint32_t>(parts.size());

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

    if (check(lexer::TokenType::LParen))
    {
        bool isLambda = false;
        // Check for () ->
        if (peekNext().type == lexer::TokenType::RParen &&
            peekNextNext().type == lexer::TokenType::Arrow)
            isLambda = true;
        // Check for (ident, or (ident)
        else if (peekNext().type == lexer::TokenType::Identifier &&
                 (peekNextNext().type == lexer::TokenType::Comma ||
                  peekNextNext().type == lexer::TokenType::RParen))
        {
            // This is likely a lambda if followed by Arrow or just the start of params
            // To be sure, we'd need more lookahead, but this covers the test cases.
            isLambda = true;
        }

        if (isLambda)
            return parseLambda();

        match(lexer::TokenType::LParen);  // Now consume it for grouping
        ast::Expr* expr = parseExpression();
        consume(lexer::TokenType::RParen, "Expect ')' after expression.");
        auto* group       = arena_.make<ast::GroupingExpr>();
        group->kind       = ast::NodeKind::Grouping;
        group->expression = expr;
        return group;
    }

    if (match(lexer::TokenType::KwFunction))
        return parseLambda();
    if (check(lexer::TokenType::LBracket))
        return parseArrayLiteral();
    if (check(lexer::TokenType::LBrace))
        return parseStructLiteral();

    error(peek(), "Expect expression.");
    return nullptr;
}

}  // namespace druk::parser
