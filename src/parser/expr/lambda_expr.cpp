#include "druk/parser/ast/lambda.hpp"
#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Expr* Parser::parseLambda()
{
    std::vector<lexer::Token> params;
    if (!check(lexer::TokenType::RParen))
    {
        do
        {
            params.push_back(consume(lexer::TokenType::Identifier, "Expect parameter name."));
        } while (match(lexer::TokenType::Comma));
    }
    consume(lexer::TokenType::RParen, "Expect ')' after parameters.");
    consume(lexer::TokenType::Arrow, "Expect '->' before lambda body.");

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
    expr->params     = arena_.makeArray<lexer::Token>(expr->paramCount);
    for (uint32_t i = 0; i < expr->paramCount; ++i)
    {
        expr->params[i] = params[i];
    }
    expr->body = body;

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
    expr->params     = arena_.makeArray<lexer::Token>(1);
    expr->params[0]  = name;
    expr->body       = body;

    return expr;
}

}  // namespace druk::parser
