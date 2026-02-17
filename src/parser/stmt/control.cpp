#include "druk/parser/core/parser.hpp"

namespace druk::parser {

ast::Stmt *Parser::parseIfStatement() {
  consume(lexer::TokenType::LParen, "Expect '(' after 'if'.");
  ast::Expr *condition = parseExpression();
  consume(lexer::TokenType::RParen, "Expect ')' after if condition.");

  ast::Stmt *thenBranch = parseBlock();
  ast::Stmt *elseBranch = nullptr;

  if (match(lexer::TokenType::KwElse)) {
    elseBranch = parseBlock();
  }

  auto *stmt = arena_.make<ast::IfStmt>();
  stmt->kind = ast::NodeKind::If;
  stmt->condition = condition;
  stmt->thenBranch = thenBranch;
  stmt->elseBranch = elseBranch;
  return stmt;
}

ast::Stmt *Parser::parseLoopStatement() {
  consume(lexer::TokenType::LParen, "Expect '(' after 'loop'.");
  ast::Expr *condition = parseExpression();
  consume(lexer::TokenType::RParen, "Expect ')' after loop condition.");

  ast::Stmt *body = parseBlock();

  auto *stmt = arena_.make<ast::LoopStmt>();
  stmt->kind = ast::NodeKind::Loop;
  stmt->condition = condition;
  stmt->body = body;
  return stmt;
}

ast::Stmt *Parser::parseReturnStatement() {
  lexer::Token keyword = previous();
  ast::Expr *value = nullptr;
  if (!check(lexer::TokenType::Semicolon)) {
    value = parseExpression();
  }

  consume(lexer::TokenType::Semicolon, "Expect ';' after return value.");

  auto *stmt = arena_.make<ast::ReturnStmt>();
  stmt->kind = ast::NodeKind::Return;
  stmt->token = keyword;
  stmt->value = value;
  return stmt;
}

} // namespace druk::parser
