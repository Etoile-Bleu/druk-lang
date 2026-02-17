#pragma once

namespace druk::parser::ast {

struct BlockStmt;
struct IfStmt;
struct LoopStmt;
struct ReturnStmt;
struct PrintStmt;
struct ExpressionStmt;
struct VarDecl;
struct FuncDecl;

struct BinaryExpr;
struct UnaryExpr;
struct LiteralExpr;
struct VariableExpr;
struct GroupingExpr;
struct AssignmentExpr;
struct CallExpr;
struct LogicalExpr;
struct ArrayLiteralExpr;
struct IndexExpr;
struct StructLiteralExpr;
struct MemberAccessExpr;

struct Stmt;
struct Expr;

class Visitor {
public:
  virtual ~Visitor() = default;

  virtual void visit(Stmt *stmt) = 0;
  virtual void visit(Expr *expr) = 0;

  virtual void visitBlock(BlockStmt *stmt) = 0;
  virtual void visitIf(IfStmt *stmt) = 0;
  virtual void visitLoop(LoopStmt *stmt) = 0;
  virtual void visitReturn(ReturnStmt *stmt) = 0;
  virtual void visitPrint(PrintStmt *stmt) = 0;
  virtual void visitExprStmt(ExpressionStmt *stmt) = 0;
  virtual void visitVar(VarDecl *stmt) = 0;
  virtual void visitFunc(FuncDecl *stmt) = 0;

  virtual void visitBinary(BinaryExpr *expr) = 0;
  virtual void visitUnary(UnaryExpr *expr) = 0;
  virtual void visitLiteral(LiteralExpr *expr) = 0;
  virtual void visitVariable(VariableExpr *expr) = 0;
  virtual void visitGrouping(GroupingExpr *expr) = 0;
  virtual void visitAssignment(AssignmentExpr *expr) = 0;
  virtual void visitCall(CallExpr *expr) = 0;
  virtual void visitLogical(LogicalExpr *expr) = 0;
  virtual void visitArrayLiteral(ArrayLiteralExpr *expr) = 0;
  virtual void visitIndex(IndexExpr *expr) = 0;
  virtual void visitStructLiteral(StructLiteralExpr *expr) = 0;
  virtual void visitMemberAccess(MemberAccessExpr *expr) = 0;
};

} // namespace druk::parser::ast
