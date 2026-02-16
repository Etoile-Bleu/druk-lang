#include "druk/semantic/analyzer.hpp"
#include <iostream>

namespace druk {

SemanticAnalyzer::SemanticAnalyzer(SymbolTable &table, ErrorReporter &errors,
                                   StringInterner &interner,
                                   std::string_view source)
    : table_(table), errors_(errors), interner_(interner), source_(source) {}

void SemanticAnalyzer::analyze(const std::vector<Stmt *> &statements) {
  for (auto *stmt : statements) {
    visit(stmt);
  }
}

void SemanticAnalyzer::visit(Stmt *stmt) {
  if (!stmt)
    return;
  switch (stmt->kind) {
  case NodeKind::Block:
    visit_block(static_cast<BlockStmt *>(stmt));
    break;
  case NodeKind::If:
    visit_if(static_cast<IfStmt *>(stmt));
    break;
  case NodeKind::Function:
    visit_func(static_cast<FuncDecl *>(stmt));
    break;
  case NodeKind::Variable:
    visit_var(static_cast<VarDecl *>(stmt));
    break;
  case NodeKind::ExpressionStmt:
    visit_expr_stmt(static_cast<ExpressionStmt *>(stmt));
    break;
  case NodeKind::Print:
    visit_print(static_cast<PrintStmt *>(stmt));
    break;
  case NodeKind::Return:
    visit_return(static_cast<ReturnStmt *>(stmt));
    break;
  case NodeKind::Loop:
    visit_loop(static_cast<LoopStmt *>(stmt));
    break;
  default:
    break;
  }
}

void SemanticAnalyzer::visit(Expr *expr) {
  if (!expr)
    return;
  switch (expr->kind) {
  case NodeKind::Binary:
    visit_binary(static_cast<BinaryExpr *>(expr));
    break;
  case NodeKind::Unary:
    visit_unary(static_cast<UnaryExpr *>(expr));
    break;
  case NodeKind::Logical:
    visit_logical(static_cast<LogicalExpr *>(expr));
    break;
  case NodeKind::Literal:
    visit_literal(static_cast<LiteralExpr *>(expr));
    break;
  case NodeKind::VariableExpr:
    visit_variable(static_cast<VariableExpr *>(expr));
    break;
  case NodeKind::Assignment:
    visit_assignment(static_cast<AssignmentExpr *>(expr));
    break;
  case NodeKind::Grouping:
    visit_grouping(static_cast<GroupingExpr *>(expr));
    break;
  case NodeKind::Call:
    visit_call(static_cast<CallExpr *>(expr));
    break;
  case NodeKind::ArrayLiteral:
  case NodeKind::StructLiteral:
  case NodeKind::Index:
  case NodeKind::MemberAccess:
    // Collections: assign Int type for now (proper type checking TODO)
    expr->type = Type::Int();
    break;
  default:
    break;
  }
}

} // namespace druk
