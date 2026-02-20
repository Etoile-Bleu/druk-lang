#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Stmt* Parser::parseForStatement()
{
    consume(lexer::TokenType::LParen, "Expect '(' after for.");

    ast::Stmt* init = nullptr;
    if (match(lexer::TokenType::KwNumber) || match(lexer::TokenType::KwString) ||
        match(lexer::TokenType::KwBoolean))
    {
        init = parseVarDeclaration();
    }
    else if (!check(lexer::TokenType::Semicolon))
    {
        init = parseExpressionStatement();
    }
    else
    {
        consume(lexer::TokenType::Semicolon, "Expect ';'.");
    }

    ast::Expr* condition = nullptr;
    if (!check(lexer::TokenType::Semicolon))
        condition = parseExpression();
    consume(lexer::TokenType::Semicolon, "Expect ';' after for condition.");

    ast::Expr* step = nullptr;
    if (!check(lexer::TokenType::RParen))
        step = parseExpression();
    consume(lexer::TokenType::RParen, "Expect ')' after for clauses.");

    ast::Stmt* body = parseBlock();

    auto* stmt      = arena_.make<ast::ForStmt>();
    stmt->kind      = ast::NodeKind::For;
    stmt->init      = init;
    stmt->condition = condition;
    stmt->step      = step;
    stmt->body      = body;
    return stmt;
}

}  // namespace druk::parser
