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
      // VariableExpr* name_expr = static_cast<VariableExpr*>(expr);
      // We need the token from VariableExpr to put into AssignmentExpr?
      // Current AssignmentExpr struct has 'Token name'. VariableExpr struct
      // has... wait, let's check VariableExpr.
      auto *assign = arena_.make<AssignmentExpr>();
      assign->kind = NodeKind::Assignment;
      // assign->name = name->token; ??? VariableExpr doesn't hold token in my
      // struct yet
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

Expr *Parser::parse_equality() { return parse_term(); }   // Shortcut
Expr *Parser::parse_comparison() { return parse_term(); } // Shortcut
Expr *Parser::parse_term() { return parse_factor(); }     // Shortcut
Expr *Parser::parse_factor() { return parse_unary(); }    // Shortcut

Expr *Parser::parse_unary() {
  if (match(TokenKind::Bang) || match(TokenKind::Minus)) {
    auto *expr = arena_.make<UnaryExpr>();
    expr->kind = NodeKind::Unary;
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
    auto *var = arena_.make<VariableExpr>();
    var->kind = NodeKind::VariableExpr;
    // var->name = previous(); // Need to add token to VariableExpr struct in
    // expr.hpp if we want it
    return var;
  }

  // Error handling if we expect an expression but find nothing
  error(peek(), "Expect expression.");
  return nullptr;
}

} // namespace druk
