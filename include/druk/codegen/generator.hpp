#pragma once
#include "druk/codegen/chunk.hpp"
#include "druk/codegen/obj.hpp"
#include "druk/common/allocator.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/semantic/analyzer.hpp"
#include <memory>

namespace druk {

class Generator {
public:
  Generator(Chunk &chunk, StringInterner &interner, ErrorReporter &errors,
            std::string_view source);

  bool generate(const std::vector<Stmt *> &statements);
  std::shared_ptr<ObjFunction> function() const { return root_function_; }

  // Visitor methods (could reuse Visitor pattern if we had one,
  // but for now manual dispatch like Analyzer)
  void visit(Stmt *stmt);
  void visit(Expr *expr);

private:
  struct Local {
    std::string_view name;
    int depth;
  };

  struct Compiler {
    std::shared_ptr<ObjFunction> function;
    std::vector<Local> locals;
    int scope_depth = 0;
    Compiler *enclosing = nullptr;
  };

  void visit_block(BlockStmt *stmt);
  void visit_if(IfStmt *stmt);
  void visit_loop(LoopStmt *stmt);
  void visit_return(ReturnStmt *stmt);
  void visit_expr_stmt(ExpressionStmt *stmt);
  void visit_var(VarDecl *stmt);
  void visit_print(PrintStmt *stmt);
  void visit_func(FuncDecl *stmt);

  void visit_binary(BinaryExpr *expr);
  void visit_unary(UnaryExpr *expr);
  void visit_literal(LiteralExpr *expr);
  void visit_grouping(GroupingExpr *expr);
  void visit_variable(VariableExpr *expr);
  void visit_assign(AssignmentExpr *expr);
  void visit_call(CallExpr *expr);
  void visit_logical(LogicalExpr *expr);
  
  // Collections
  void visit_array_literal(ArrayLiteralExpr *expr);
  void visit_index(IndexExpr *expr);
  void visit_index_set(IndexExpr *lhs, Expr *rhs);
  void visit_struct_literal(StructLiteralExpr *expr);
  void visit_member_access(MemberAccessExpr *expr);
  void visit_member_set(MemberAccessExpr *lhs, Expr *rhs);

  void emit_byte(uint8_t byte);
  void emit_opcode(OpCode opcode);
  void emit_constant(Value value);

  // Jump patching
  int emit_jump(OpCode opcode);
  void patch_jump(int offset);
  void emit_loop(int loop_start);

  // Local variables
  // struct Local moved to top

  void begin_scope();
  void end_scope();
  void add_local(std::string_view name);
  int resolve_local(std::string_view name);

  void init_compiler(struct Compiler *compiler, int type);
  void end_compiler(); // If used

  // Chunk &chunk_; // Use current_chunk() instead

  Compiler *enclosing = nullptr;

  Compiler *current_compiler_ = nullptr;

  Chunk &output_chunk_; // The chunk passed by caller, to be filled at end
  StringInterner &interner_;
  ErrorReporter &errors_;
  std::string_view source_;
  int line_ = 0; // Current line for debug info

  Chunk &current_chunk();
  std::shared_ptr<ObjFunction> root_function_;
};

} // namespace druk
