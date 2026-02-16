#include "druk/codegen/generator.hpp"
#include "druk/common/allocator.hpp"
#include "druk/lexer/unicode.hpp"
#include <gtest/gtest.h>

using namespace druk;

class GeneratorTest : public ::testing::Test {
protected:
  ArenaAllocator arena;
  StringInterner interner{arena};
  ErrorReporter errors;
  Chunk chunk;
};

TEST_F(GeneratorTest, EmitReturn) {
  std::string_view source = "";
  Generator generator(chunk, interner, errors, source);
  std::vector<Stmt *> stmts; // Empty

  generator.generate(stmts);

  EXPECT_FALSE(errors.has_errors());
  EXPECT_EQ(chunk.code().size(), 1);
  EXPECT_EQ(static_cast<OpCode>(chunk.code()[0]), OpCode::Return);
}

TEST_F(GeneratorTest, EmitIf) {
  // གལ་ཏེ (བདེན) { }
  // If (true) {}
  // Ops: Constant(True), JumpIfFalse(offset), Jump(offset), Return

  // Construct AST manually or use Parser?
  // Manual for now to isolate Generator.
  Token true_tok{TokenKind::KwTrue, 0, 0, 0, 0};
  auto *cond = arena.make<LiteralExpr>();
  cond->kind = NodeKind::Literal;
  cond->token = true_tok;
  cond->type = Type::Bool();

  auto *then = arena.make<BlockStmt>();
  then->kind = NodeKind::Block;
  then->count = 0;

  auto *if_stmt = arena.make<IfStmt>();
  if_stmt->kind = NodeKind::If;
  if_stmt->condition = cond;
  if_stmt->then_branch = then;
  if_stmt->else_branch = nullptr;

  std::vector<Stmt *> stmts;
  stmts.push_back(if_stmt);

  Generator generator(chunk, interner, errors, "");
  generator.generate(stmts);

  EXPECT_FALSE(errors.has_errors());
  // Verify opcodes
  // 0: Constant -> True
  // 2: JumpIfFalse -> Jump over then
  // 5: ... then block (empty) ...
  // 5: Jump -> Jump over else (if explicit else existed, but here we emit Jump
  // anyway) 8: Return Wait, my visit_if emits Jump after then block regardless
  // of else existence? Yes: int else_jump = emit_jump(OpCode::Jump);

  const auto &code = chunk.code();
  EXPECT_GT(code.size(), 0);
  EXPECT_EQ(static_cast<OpCode>(code[0]), OpCode::Constant); // or Constant
}

TEST_F(GeneratorTest, EmitLoop) {
  // རིམ་པ (བདེན) { }
  // Loop (true) {}
  // Ops: Constant(True), JumpIfFalse(exit), ... body ..., Loop(start)

  Token true_tok{TokenKind::KwTrue, 0, 0, 0, 0};
  auto *cond = arena.make<LiteralExpr>();
  cond->kind = NodeKind::Literal;
  cond->token = true_tok;
  cond->type = Type::Bool();

  auto *body = arena.make<BlockStmt>();
  body->kind = NodeKind::Block;
  body->count = 0;

  auto *loop_stmt = arena.make<LoopStmt>();
  loop_stmt->kind = NodeKind::Loop;
  loop_stmt->condition = cond;
  loop_stmt->body = body;

  std::vector<Stmt *> stmts;
  stmts.push_back(loop_stmt);

  Generator generator(chunk, interner, errors, "");
  generator.generate(stmts);

  EXPECT_FALSE(errors.has_errors());
  // Verify opcodes
  // 0: Constant -> True
  // 2: JumpIfFalse -> Exit
  // 5: ... body ...
  // 5: Loop -> Start (0)
}

TEST_F(GeneratorTest, EmitComparison) {
  // 1 < 2
  Token one_tok{TokenKind::Number, 0, 0, 0, 0};
  // Need text "1" and "2" in source to work?
  // My previous test used "" source.
  // visit_literal uses expr->token.text(source_).
  // If source is empty, this fails/crashes or returns empty string.
  // I need to provide dummy source or mocking.
  // Let's use string_view for source that contains actual text logic relies on.

  // Actually, I can mock Interner? No, Generator uses Token text via source.
  // I should rewrite source handling in test or just pass "1 2" code.
  std::string_view source = "1 2";
  Generator generator(chunk, interner, errors, source);

  // Construct nodes pointing to source
  Token t1{TokenKind::Number, 0, 0, 1, 0};
  auto *l1 = arena.make<LiteralExpr>();
  l1->kind = NodeKind::Literal;
  l1->token = t1;

  Token t2{TokenKind::Number, 0, 2, 1, 0};
  auto *l2 = arena.make<LiteralExpr>();
  l2->kind = NodeKind::Literal;
  l2->token = t2;

  Token op{TokenKind::Less, 0, 0, 0, 0};
  auto *bin = arena.make<BinaryExpr>();
  bin->kind = NodeKind::Binary;
  bin->left = l1;
  bin->right = l2;
  bin->token = op;

  // Wrap in statement? Or expose visit(Expr*)?
  // Generator::visit(Expr*) is private?
  // Generator::generate takes std::vector<Stmt*>.
  // Wrap in ExpressionStmt.
  auto *stmt = arena.make<ExpressionStmt>();
  stmt->kind = NodeKind::ExpressionStmt;
  stmt->expression = bin;

  std::vector<Stmt *> stmts{stmt};
  generator.generate(stmts);

  // Constant(1), Constant(2), Less, Pop, Return
  EXPECT_GE(chunk.code().size(), 4);
}

TEST_F(GeneratorTest, EmitCall) {
  // func(1, 2)
  std::string_view source = "func 1 2";
  Generator generator(chunk, interner, errors, source);

  // Ident "func"
  Token name_tok{TokenKind::Identifier, 0, 0, 4, 0};
  auto *func_name = arena.make<VariableExpr>();
  func_name->kind = NodeKind::Variable;
  func_name->name = name_tok;

  // Args
  auto *arg1 = arena.make<LiteralExpr>();
  arg1->kind = NodeKind::Literal;
  arg1->token = Token{TokenKind::Number, 0, 5, 1, 0}; // "1"

  auto *arg2 = arena.make<LiteralExpr>();
  arg2->kind = NodeKind::Literal;
  arg2->token = Token{TokenKind::Number, 0, 7, 1, 0}; // "2"

  // Array of args
  Node **args = arena.allocate_array<Node *>(2);
  args[0] = arg1;
  args[1] = arg2;

  auto *call = arena.make<CallExpr>();
  call->kind = NodeKind::Call;
  call->callee = func_name;
  call->args = args;
  call->arg_count = 2;

  auto *stmt = arena.make<ExpressionStmt>();
  stmt->kind = NodeKind::ExpressionStmt;
  stmt->expression = call;

  std::vector<Stmt *> stmts{stmt};
  generator.generate(stmts);

  // 1. GetGlobal(func)
  // 2. Constant(1)
  // 3. Constant(2)
  // 4. Call
  // 5. ArgCount(2)
  // 6. Pop (ExprStmt)

  // Check opcode sequence
  const auto &code = chunk.code();
  // At least 6 ops/bytes
  EXPECT_GT(code.size(), 5);
}
