#include "druk/parser/ast/lambda.hpp"
#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Expr* Parser::parseLambda()
{
    std::vector<ast::Param> params;
    if (match(lexer::TokenType::LParen))
    {
        if (!check(lexer::TokenType::RParen))
        {
            do
            {
                ast::Type* type = nullptr;
                if (check(lexer::TokenType::KwNumber) || check(lexer::TokenType::KwString) ||
                    check(lexer::TokenType::KwBoolean) || check(lexer::TokenType::KwVoid) ||
                    check(lexer::TokenType::LParen) || check(lexer::TokenType::KwFunction))
                {
                    type = parseType();
                }
                lexer::Token name = consume(lexer::TokenType::Identifier, "Expect parameter name.");
                params.push_back({type, name});
            } while (match(lexer::TokenType::Comma));
        }
        consume(lexer::TokenType::RParen, "Expect ')' after parameters.");
    }
    else
    {
        // JS-style without parens? (already handled by parsePrimary usually)
    }

    ast::Type* returnType = nullptr;
    if (match(lexer::TokenType::Arrow))
    {
        // In Druk, if -> is followed by a type keyword AND THEN a brace, it's a return type.
        // Otherwise, it's the start of an expression body (like in (a, b) -> a + b).
        // Let's check for type keywords.
        if (check(lexer::TokenType::KwNumber) || check(lexer::TokenType::KwString) ||
            check(lexer::TokenType::KwBoolean) || check(lexer::TokenType::KwVoid) ||
            check(lexer::TokenType::LParen))
        {
            // If it's a LParen, it could be a function type or a grouping.
            // In lambdas, we prefer treating ( ... ) after -> as a return type ONLY IF followed by
            // { But wait, the most common case in tests is omitted return type and Arrow ->
            // Expression.

            // To pass the tests (a, b) -> a + b:
            // We only parseType if we are SURE it's not the start of an expression.
            // But 'a' is an identifier.

            // Actually, if we are in this case, we can try to peekNext.
            // If peek().type is a type keyword and peekNext() is { , then it's a return type.
            if (peekNext().type == lexer::TokenType::LBrace)
            {
                returnType = parseType();
            }
        }
    }

    ast::Stmt* body;
    if (check(lexer::TokenType::LBrace))
    {
        body = parseBlock();
    }
    else
    {
        ast::Expr* expr  = parseExpression();
        auto*      stmt  = arena_.make<ast::ExpressionStmt>();
        stmt->kind       = ast::NodeKind::ExpressionStmt;
        stmt->expression = expr;
        body             = stmt;
    }

    auto* expr       = arena_.make<ast::LambdaExpr>();
    expr->kind       = ast::NodeKind::Lambda;
    expr->paramCount = static_cast<uint32_t>(params.size());
    expr->params     = arena_.allocateArray<ast::Param>(expr->paramCount);
    for (uint32_t i = 0; i < expr->paramCount; ++i)
    {
        expr->params[i] = params[i];
    }
    expr->returnType = returnType;
    expr->body       = body;

    return expr;
}

ast::Expr* Parser::parseLambdaFromIdentifier(lexer::Token name)
{
    ast::Stmt* body;
    if (check(lexer::TokenType::LBrace))
    {
        body = parseBlock();
    }
    else
    {
        ast::Expr* expr  = parseExpression();
        auto*      stmt  = arena_.make<ast::ExpressionStmt>();
        stmt->kind       = ast::NodeKind::ExpressionStmt;
        stmt->expression = expr;
        body             = stmt;
    }

    auto* expr       = arena_.make<ast::LambdaExpr>();
    expr->kind       = ast::NodeKind::Lambda;
    expr->paramCount = 1;
    expr->params     = arena_.allocateArray<ast::Param>(1);
    expr->params[0]  = {nullptr, name};
    expr->body       = body;

    return expr;
}

}  // namespace druk::parser
