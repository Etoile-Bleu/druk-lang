#include "druk/parser/core/parser.hpp"

namespace druk {

Expr *Parser::parse_expression() { return parse_assignment(); }

Expr *Parser::parse_assignment() {
  Expr *expr = parse_logic_or();
  // Assignment logic here
  if (match(TokenKind::Equal)) {
    // Validation: expr must be lvalue (VariableExpr)
    Expr *value = parse_assignment();

    if (expr->kind == NodeKind::VariableExpr) {
      auto *assign = arena_.make<AssignmentExpr>();
      assign->kind = NodeKind::Assignment;
      assign->name = expr->token;  // Get token from the variable expression
      assign->value = value;
      return assign;
    }
    error(previous(), "Invalid assignment target.");
  }
  return expr;
}

Expr *Parser::parse_logic_or() {
  Expr *expr = parse_logic_and();
  while (match(TokenKind::Or)) {
    Expr *right = parse_logic_and();
    auto *node = arena_.make<LogicalExpr>();
    node->kind = NodeKind::Logical;
    node->left = expr;
    node->right = right;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_logic_and() {
  Expr *expr = parse_equality();
  while (match(TokenKind::And)) {
    Expr *right = parse_equality();
    auto *node = arena_.make<LogicalExpr>();
    node->kind = NodeKind::Logical;
    node->left = expr;
    node->right = right;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_equality() {
  Expr *expr = parse_comparison();
  while (match(TokenKind::EqualEqual) || match(TokenKind::BangEqual)) {
    Token op = previous();
    Expr *right = parse_comparison();
    auto *binary = arena_.make<BinaryExpr>();
    binary->kind = NodeKind::Binary;
    binary->token = op;
    binary->left = expr;
    binary->right = right;
    expr = binary;
  }
  return expr;
}

Expr *Parser::parse_comparison() {
  Expr *expr = parse_term();
  while (match(TokenKind::Less) || match(TokenKind::LessEqual) ||
         match(TokenKind::Greater) || match(TokenKind::GreaterEqual)) {
    Token op = previous();
    Expr *right = parse_term();
    auto *binary = arena_.make<BinaryExpr>();
    binary->kind = NodeKind::Binary;
    binary->token = op;
    binary->left = expr;
    binary->right = right;
    expr = binary;
  }
  return expr;
}

Expr *Parser::parse_term() {
  Expr *expr = parse_factor();
  while (match(TokenKind::Plus) || match(TokenKind::Minus)) {
    Token op = previous();
    Expr *right = parse_factor();
    auto *binary = arena_.make<BinaryExpr>();
    binary->kind = NodeKind::Binary;
    binary->token = op;
    binary->left = expr;
    binary->right = right;
    expr = binary;
  }
  return expr;
}

Expr *Parser::parse_factor() {
  Expr *expr = parse_unary();
  while (match(TokenKind::Star) || match(TokenKind::Slash)) {
    Token op = previous();
    Expr *right = parse_unary();
    auto *binary = arena_.make<BinaryExpr>();
    binary->kind = NodeKind::Binary;
    binary->token = op;
    binary->left = expr;
    binary->right = right;
    expr = binary;
  }
  return expr;
}

Expr *Parser::parse_unary() {
  if (match(TokenKind::Bang) || match(TokenKind::Minus)) {
    Token op = previous();
    auto *expr = arena_.make<UnaryExpr>();
    expr->kind = NodeKind::Unary;
    expr->token = op;
    expr->right = parse_unary();
    return expr;
  }
  return parse_call();
}

Expr *Parser::parse_call() { return parse_primary(); } // Shortcut

Expr *Parser::parse_primary() {
  if (match(TokenKind::Number) || match(TokenKind::String) ||
      match(TokenKind::KwTrue) || match(TokenKind::KwFalse)) {
    auto *lit = arena_.make<LiteralExpr>();
    lit->kind = NodeKind::Literal;
    lit->token = previous();
    return lit;
  }
  if (match(TokenKind::LParen)) {
    Expr *expr = parse_expression();
    consume(TokenKind::RParen, "Expect ')' after expression.");
    auto *group = arena_.make<GroupingExpr>();
    group->kind = NodeKind::Grouping;
    group->expression = expr;
    return group;
  }
  if (match(TokenKind::Identifier)) {
    Token name = previous();
    auto *var = arena_.make<VariableExpr>();
    var->kind = NodeKind::VariableExpr;
    var->token = name;
    return var;
  }

  // Error handling if we expect an expression but find nothing
  error(peek(), "Expect expression.");
  return nullptr;
}

} // namespace druk
