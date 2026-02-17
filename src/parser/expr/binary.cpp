#include "druk/parser/core/parser.hpp"

namespace druk::parser {

ast::Expr *Parser::parseAssignment() {
  ast::Expr *expr = parseLogicalOr();

  if (match(lexer::TokenType::Equal)) {
    lexer::Token equals = previous();
    ast::Expr *value = parseAssignment();

    if (expr->kind == ast::NodeKind::VariableExpr ||
        expr->kind == ast::NodeKind::Index ||
        expr->kind == ast::NodeKind::MemberAccess) {
      auto *assign = arena_.make<ast::AssignmentExpr>();
      assign->kind = ast::NodeKind::Assignment;
      assign->target = expr;
      assign->value = value;
      assign->token = equals;
      return assign;
    }

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

ast::Expr *Parser::parseLogicalOr() {
  ast::Expr *expr = parseLogicalAnd();

  while (match(lexer::TokenType::Or)) {
    lexer::Token op = previous();
    ast::Expr *right = parseLogicalAnd();
    auto *binary = arena_.make<ast::BinaryExpr>();
    binary->kind = ast::NodeKind::Logical;
    binary->left = expr;
    binary->right = right;
    binary->token = op;
    expr = binary;
  }

  return expr;
}

ast::Expr *Parser::parseLogicalAnd() {
  ast::Expr *expr = parseEquality();

  while (match(lexer::TokenType::And)) {
    lexer::Token op = previous();
    ast::Expr *right = parseEquality();
    auto *binary = arena_.make<ast::BinaryExpr>();
    binary->kind = ast::NodeKind::Logical;
    binary->left = expr;
    binary->right = right;
    binary->token = op;
    expr = binary;
  }

  return expr;
}

ast::Expr *Parser::parseEquality() {
  ast::Expr *expr = parseComparison();

  while (match(lexer::TokenType::EqualEqual) || match(lexer::TokenType::BangEqual)) {
    lexer::Token op = previous();
    ast::Expr *right = parseComparison();
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
