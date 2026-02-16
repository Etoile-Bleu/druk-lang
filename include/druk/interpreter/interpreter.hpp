#pragma once

#include "druk/common/allocator.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/stmt.hpp"
#include <string_view>
#include <unordered_map>
#include <variant>

namespace druk {

// Efficient value representation using NaN-boxing could be added later
// For now, use std::variant for simplicity
class Value {
public:
  using Variant = std::variant<double, bool, std::string_view>;
  
  Value() : data_(0.0) {}
  explicit Value(double d) : data_(d) {}
  explicit Value(bool b) : data_(b) {}
  explicit Value(std::string_view s) : data_(s) {}
  
  bool is_number() const { return std::holds_alternative<double>(data_); }
  bool is_bool() const { return std::holds_alternative<bool>(data_); }
  bool is_string() const { return std::holds_alternative<std::string_view>(data_); }
  
  double as_number() const { return std::get<double>(data_); }
  bool as_bool() const { return std::get<bool>(data_); }
  std::string_view as_string() const { return std::get<std::string_view>(data_); }
  
  bool is_truthy() const {
    if (is_bool()) return as_bool();
    if (is_number()) return as_number() != 0.0;
    return !is_string() || !as_string().empty();
  }
  
private:
  Variant data_;
};

class Interpreter {
public:
  Interpreter(std::string_view source) : source_(source) {}
  
  void execute(const std::vector<Stmt*>& statements);
  
private:
  Value evaluate(Expr* expr);
  void execute_stmt(Stmt* stmt);
  
  Value visit_binary(BinaryExpr* expr);
  Value visit_unary(UnaryExpr* expr);
  Value visit_literal(LiteralExpr* expr);
  Value visit_grouping(GroupingExpr* expr);
  Value visit_variable(VariableExpr* expr);
  Value visit_assignment(AssignmentExpr* expr);
  Value visit_logical(LogicalExpr* expr);
  
  void visit_expression_stmt(ExpressionStmt* stmt);
  void visit_var_decl(VarDecl* stmt);
  void visit_block(BlockStmt* stmt);
  void visit_if(IfStmt* stmt);
  void visit_loop(LoopStmt* stmt);
  
  std::string_view source_;
  
  // Simple flat environment for now - could optimize with scopes later
  std::unordered_map<std::string_view, Value> variables_;
};

} // namespace druk
