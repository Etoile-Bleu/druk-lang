#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Stmt* Parser::parseWhileStatement()
{
    consume(lexer::TokenType::LParen, "Expect '(' after while.");
    ast::Expr* condition = parseExpression();
    consume(lexer::TokenType::RParen, "Expect ')' after while condition.");

    ast::Stmt* body = parseStatement();

    auto* stmt      = arena_.make<ast::WhileStmt>();
    stmt->kind      = ast::NodeKind::While;
    stmt->condition = condition;
    stmt->body      = body;
    return stmt;
}

}  // namespace druk::parser
