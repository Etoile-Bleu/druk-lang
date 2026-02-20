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
                ast::Type*   type = parseType();
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
        returnType = parseType();

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
