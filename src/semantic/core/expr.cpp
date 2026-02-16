#include "druk/semantic/analyzer.hpp"

namespace druk {

void SemanticAnalyzer::visit_binary(BinaryExpr *expr) {
  visit(expr->left);
  visit(expr->right);

  if (expr->left->type == Type::Error() || expr->right->type == Type::Error()) {
    expr->type = Type::Error();
    return;
  }

  bool is_comparison = false;
  bool is_logical = false;
  switch (expr->token.kind) {
  case TokenKind::EqualEqual:
  case TokenKind::BangEqual:
  case TokenKind::Less:
  case TokenKind::LessEqual:
  case TokenKind::Greater:
  case TokenKind::GreaterEqual:
    is_comparison = true;
    break;
  case TokenKind::And:
  case TokenKind::Or:
    is_logical = true;
    break;
  default:
    break;
  }

  if (is_comparison) {
    // Both sides must be same type for equality, or both numbers for order
    if (expr->left->type == expr->right->type) {
      expr->type = Type::Bool();
    } else {
      expr->type = Type::Error();
    }
  } else if (is_logical) {
    // Logical operators: both operands must be Bool -> Bool
    if (expr->left->type == Type::Bool() && expr->right->type == Type::Bool()) {
      expr->type = Type::Bool();
    } else {
      expr->type = Type::Error();
    }
  } else {
    // Arithmetic: only Int (+|-|*|/) Int -> Int
    if (expr->left->type == Type::Int() && expr->right->type == Type::Int()) {
      expr->type = Type::Int();
    } else {
      expr->type = Type::Error();
    }
  }
}

void SemanticAnalyzer::visit_logical(LogicalExpr *expr) {
  visit(expr->left);
  visit(expr->right);

  // Logical operators: both operands must be Bool -> Bool
  if (expr->left->type == Type::Error() || expr->right->type == Type::Error()) {
    expr->type = Type::Error();
    return;
  }

  if (expr->left->type == Type::Bool() && expr->right->type == Type::Bool()) {
    expr->type = Type::Bool();
  } else {
    expr->type = Type::Error();
  }
}

void SemanticAnalyzer::visit_unary(UnaryExpr *expr) {
  visit(expr->right);
  // Logical NOT (!) expects Bool -> Bool
  if (expr->token.kind == TokenKind::Bang) {
    if (expr->right->type == Type::Bool()) {
      expr->type = Type::Bool();
    } else {
      expr->type = Type::Error();
    }
  }
  // Unary minus (-) expects Int -> Int
  else if (expr->token.kind == TokenKind::Minus) {
    if (expr->right->type == Type::Int()) {
      expr->type = Type::Int();
    } else {
      expr->type = Type::Error();
    }
  } else {
    expr->type = Type::Error();
  }
}

void SemanticAnalyzer::visit_literal(LiteralExpr *expr) {
  // Determine type from token kind
  switch (expr->token.kind) {
  case TokenKind::Number:
    expr->type = Type::Int();
    break;
  case TokenKind::String:
    expr->type = Type::String();
    break;
  case TokenKind::KwTrue:
  case TokenKind::KwFalse:
  case TokenKind::KwBoolean:
    expr->type = Type::Bool();
    break;
  default:
    expr->type = Type::Error();
    break;
  }
}

void SemanticAnalyzer::visit_variable(VariableExpr *expr) {
  std::string_view name = expr->name.text(source_);
  
  // Check for built-in functions
  if (name == "ཚད་" || name == "སྣོན་" || name == "ཕྱིར་ལེན་" ||
      name == "རིགས་" || name == "མིང་ཐོ་" || name == "བེད་སྤྱོད་" ||
      name == "ཡོད་" || name == "ནང་འཇུག་" ||
      name == "argv" || name == "argc" ||
      name == "ནང་འཇུག་ཐོ་" || name == "ནང་འཇུག་གྲངས་") {
    expr->type = Type::Int(); // Dummy type for built-ins
    return;
  }
  
  Symbol *sym = table_.resolve(name);
  if (!sym) {
    errors_.report(
        Error{ErrorLevel::Error,
              {expr->name.line, 0, expr->name.offset, expr->name.length},
              "Undefined variable '" + std::string(name) + "'.",
              ""});
    expr->type = Type::Error();
  } else {
    expr->type = sym->type;
  }
}

void SemanticAnalyzer::visit_assignment(AssignmentExpr *expr) {
  visit(expr->value);
  visit(expr->target);
  
  // For now, just propagate the value type
  // TODO: proper lvalue type checking
  if (expr->target->type == Type::Error() || expr->value->type == Type::Error()) {
    expr->type = Type::Error();
  } else {
    expr->type = expr->value->type;
  }
}

void SemanticAnalyzer::visit_grouping(GroupingExpr *expr) {
  visit(expr->expression);
  expr->type = expr->expression->type;
}

void SemanticAnalyzer::visit_call(CallExpr *expr) {
  visit(expr->callee);

  for (uint32_t i = 0; i < expr->arg_count; ++i) {
    visit(static_cast<Expr *>(expr->args[i]));
  }

  // Assume functions return Int by default
  expr->type = Type::Int();
}

} // namespace druk
