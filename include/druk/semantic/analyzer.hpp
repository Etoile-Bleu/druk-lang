#pragma once

#include "druk/common/allocator.hpp"
#include "druk/common/error.hpp"
#include "druk/lexer/unicode.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/node.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/semantic/table.hpp"
#include <vector>

namespace druk {

class SemanticAnalyzer {
public:
  SemanticAnalyzer(SymbolTable &table, ErrorReporter &errors,
                   StringInterner &interner, std::string_view source);

  void analyze(const std::vector<Stmt *> &statements);

  // Visitor methods
  void visit(Stmt *stmt);
  void visit(Expr *expr);

  // Specific visits
  void visit_block(BlockStmt *stmt);
  void visit_if(IfStmt *stmt);
  void visit_func(FuncDecl *stmt);
  void visit_var(VarDecl *stmt);
  void visit_print(PrintStmt *stmt);
  void visit_expr_stmt(ExpressionStmt *stmt);
  void visit_return(ReturnStmt *stmt);
  void visit_loop(LoopStmt *stmt);

  void visit_binary(BinaryExpr *expr);
  void visit_unary(UnaryExpr *expr);
  void visit_logical(LogicalExpr *expr);
  void visit_literal(LiteralExpr *expr);
  void visit_variable(VariableExpr *expr);
  void visit_assignment(AssignmentExpr *expr);
  void visit_grouping(GroupingExpr *expr);
  void visit_call(CallExpr *expr);

private:
  SymbolTable &table_;
  ErrorReporter &errors_;
  StringInterner &interner_;
  std::string_view source_;
  // We might need to track current function return type etc.
};

} // namespace druk
