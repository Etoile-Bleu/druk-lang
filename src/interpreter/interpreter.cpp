#include "druk/interpreter/interpreter.hpp"
#include "druk/parser/ast/kinds.hpp"
#include <charconv>
#include <cmath>

namespace druk {

void Interpreter::execute(const std::vector<Stmt*>& statements) {
  for (Stmt* stmt : statements) {
    execute_stmt(stmt);
  }
}

void Interpreter::execute_stmt(Stmt* stmt) {
  if (!stmt) return;
  
  switch (stmt->kind) {
    case NodeKind::ExpressionStmt:
      visit_expression_stmt(static_cast<ExpressionStmt*>(stmt));
      break;
    case NodeKind::Variable:
      visit_var_decl(static_cast<VarDecl*>(stmt));
      break;
    case NodeKind::Block:
      visit_block(static_cast<BlockStmt*>(stmt));
      break;
    case NodeKind::If:
      visit_if(static_cast<IfStmt*>(stmt));
      break;
    case NodeKind::Loop:
      visit_loop(static_cast<LoopStmt*>(stmt));
      break;
    default:
      // Unhandled statement type
      break;
  }
}

Value Interpreter::evaluate(Expr* expr) {
  if (!expr) return Value();
  
  switch (expr->kind) {
    case NodeKind::Binary:
      return visit_binary(static_cast<BinaryExpr*>(expr));
    case NodeKind::Unary:
      return visit_unary(static_cast<UnaryExpr*>(expr));
    case NodeKind::Literal:
      return visit_literal(static_cast<LiteralExpr*>(expr));
    case NodeKind::Grouping:
      return visit_grouping(static_cast<GroupingExpr*>(expr));
    case NodeKind::VariableExpr:
      return visit_variable(static_cast<VariableExpr*>(expr));
    case NodeKind::Assignment:
      return visit_assignment(static_cast<AssignmentExpr*>(expr));
    case NodeKind::Logical:
      return visit_logical(static_cast<LogicalExpr*>(expr));
    default:
      return Value();
  }
}

Value Interpreter::visit_literal(LiteralExpr* expr) {
  Token token = expr->token;
  std::string_view text = token.text(source_);
  
  switch (token.kind) {
    case TokenKind::Number: {
      // Fast path: use std::from_chars for optimal performance
      double value;
      auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
      if (ec == std::errc()) {
        return Value(value);
      }
      return Value(0.0);
    }
    case TokenKind::String:
      return Value(text);
    case TokenKind::KwTrue:
      return Value(true);
    case TokenKind::KwFalse:
      return Value(false);
    default:
      return Value();
  }
}

Value Interpreter::visit_binary(BinaryExpr* expr) {
  Value left = evaluate(expr->left);
  Value right = evaluate(expr->right);
  
  // Optimization: inline common operations
  TokenKind op = expr->token.kind;
  
  // Fast path for number operations
  if (left.is_number() && right.is_number()) {
    double l = left.as_number();
    double r = right.as_number();
    
    switch (op) {
      case TokenKind::Plus:
        return Value(l + r);
      case TokenKind::Minus:
        return Value(l - r);
      case TokenKind::Star:
        return Value(l * r);
      case TokenKind::Slash:
        return Value(l / r);
      case TokenKind::Less:
        return Value(l < r);
      case TokenKind::LessEqual:
        return Value(l <= r);
      case TokenKind::Greater:
        return Value(l > r);
      case TokenKind::GreaterEqual:
        return Value(l >= r);
      case TokenKind::EqualEqual:
        return Value(l == r);
      case TokenKind::BangEqual:
        return Value(l != r);
      default:
        break;
    }
  }
  
  return Value();
}

Value Interpreter::visit_unary(UnaryExpr* expr) {
  Value right = evaluate(expr->right);
  
  switch (expr->token.kind) {
    case TokenKind::Minus:
      if (right.is_number()) {
        return Value(-right.as_number());
      }
      break;
    case TokenKind::Bang:
      return Value(!right.is_truthy());
    default:
      break;
  }
  
  return Value();
}

Value Interpreter::visit_grouping(GroupingExpr* expr) {
  return evaluate(expr->expression);
}

Value Interpreter::visit_variable(VariableExpr* expr) {
  std::string_view name = expr->token.text(source_);
  auto it = variables_.find(name);
  if (it != variables_.end()) {
    return it->second;
  }
  return Value();
}

Value Interpreter::visit_assignment(AssignmentExpr* expr) {
  Value value = evaluate(expr->value);
  std::string_view name = expr->name.text(source_);
  variables_[name] = value;
  return value;
}

Value Interpreter::visit_logical(LogicalExpr* expr) {
  Value left = evaluate(expr->left);
  
  // Short-circuit evaluation
  if (expr->token.kind == TokenKind::Or) {
    if (left.is_truthy()) return left;
  } else { // And
    if (!left.is_truthy()) return left;
  }
  
  return evaluate(expr->right);
}

void Interpreter::visit_expression_stmt(ExpressionStmt* stmt) {
  evaluate(stmt->expression);
}

void Interpreter::visit_var_decl(VarDecl* stmt) {
  Value value;
  if (stmt->initializer) {
    value = evaluate(stmt->initializer);
  }
  std::string_view name = stmt->name.text(source_);
  variables_[name] = value;
}

void Interpreter::visit_block(BlockStmt* stmt) {
  for (uint32_t i = 0; i < stmt->count; ++i) {
    execute_stmt(stmt->statements[i]);
  }
}

void Interpreter::visit_if(IfStmt* stmt) {
  Value condition = evaluate(stmt->condition);
  if (condition.is_truthy()) {
    execute_stmt(stmt->then_branch);
  } else if (stmt->else_branch) {
    execute_stmt(stmt->else_branch);
  }
}

void Interpreter::visit_loop(LoopStmt* stmt) {
  // Optimization: evaluate condition once per iteration
  while (true) {
    Value condition = evaluate(stmt->condition);
    if (!condition.is_truthy()) break;
    execute_stmt(stmt->body);
  }
}

} // namespace druk
