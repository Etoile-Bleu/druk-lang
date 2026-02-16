#include "druk/semantic/analyzer.hpp"

namespace druk {

void SemanticAnalyzer::visit_block(BlockStmt *stmt) {
  if (!stmt)
    return;
  table_.enter_scope();
  if (stmt->statements && stmt->count > 0) {
    for (uint32_t i = 0; i < stmt->count; ++i) {
      visit(stmt->statements[i]);
    }
  }
  table_.exit_scope();
}

void SemanticAnalyzer::visit_print(PrintStmt *stmt) { visit(stmt->expression); }

void SemanticAnalyzer::visit_if(IfStmt *stmt) {
  visit(stmt->condition);
  if (stmt->condition->type != Type::Bool() &&
      stmt->condition->type != Type::Error()) {
    errors_.report(
        Error{ErrorLevel::Error,
              {stmt->condition->token.line, 0, stmt->condition->token.offset,
               stmt->condition->token.length},
              "Condition must be a boolean.",
              ""});
  }

  visit(stmt->then_branch);
  if (stmt->else_branch)
    visit(stmt->else_branch);
}

void SemanticAnalyzer::visit_expr_stmt(ExpressionStmt *stmt) {
  visit(stmt->expression);
}

void SemanticAnalyzer::visit_return(ReturnStmt *stmt) {
  if (stmt->value) {
    visit(stmt->value);
  }
}

void SemanticAnalyzer::visit_loop(LoopStmt *stmt) {
  visit(stmt->condition);
  if (stmt->condition && stmt->condition->type != Type::Bool() &&
      stmt->condition->type != Type::Error()) {
    errors_.report(
        Error{ErrorLevel::Error,
              {stmt->condition->token.line, 0, stmt->condition->token.offset,
               stmt->condition->token.length},
              "Loop condition must be a boolean.",
              ""});
  }
  visit(stmt->body);
}

} // namespace druk
