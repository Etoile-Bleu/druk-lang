#include "druk/parser/core/parser.hpp"

namespace druk::parser {

ast::Expr *Parser::parseComparison() {
  ast::Expr *expr = parseTerm();

  while (match(lexer::TokenType::Greater) || match(lexer::TokenType::GreaterEqual) ||
         match(lexer::TokenType::Less) || match(lexer::TokenType::LessEqual)) {
    lexer::Token op = previous();
    ast::Expr *right = parseTerm();
    auto *binary = arena_.make<ast::BinaryExpr>();
    binary->kind = ast::NodeKind::Binary;
    binary->left = expr;
    binary->right = right;
    binary->token = op;
    expr = binary;
  }

  return expr;
}

ast::Expr *Parser::parseTerm() {
  ast::Expr *expr = parseFactor();

  while (match(lexer::TokenType::Minus) || match(lexer::TokenType::Plus)) {
    lexer::Token op = previous();
    ast::Expr *right = parseFactor();
    auto *binary = arena_.make<ast::BinaryExpr>();
    binary->kind = ast::NodeKind::Binary;
    binary->left = expr;
    binary->right = right;
    binary->token = op;
    expr = binary;
  }

  return expr;
}

ast::Expr *Parser::parseFactor() {
  ast::Expr *expr = parseUnary();

  while (match(lexer::TokenType::Slash) || match(lexer::TokenType::Star)) {
    lexer::Token op = previous();
    ast::Expr *right = parseUnary();
    auto *binary = arena_.make<ast::BinaryExpr>();
    binary->kind = ast::NodeKind::Binary;
    binary->left = expr;
    binary->right = right;
    binary->token = op;
    expr = binary;
  }

  return expr;
}

} // namespace druk::parser
