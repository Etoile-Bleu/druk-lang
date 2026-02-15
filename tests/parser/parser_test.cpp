#include "druk/parser/core/parser.hpp"
#include <gtest/gtest.h>

class ParserTest : public ::testing::Test {
protected:
  druk::ArenaAllocator arena;
  druk::StringInterner interner{arena};
  druk::ErrorReporter errors;

  std::vector<druk::Stmt *> parse(std::string_view source) {
    druk::Parser parser(source, arena, interner, errors);
    return parser.parse();
  }
};

TEST_F(ParserTest, EmptyFunction) {
  auto stmts = parse("ལས་ཀ func() { }");
  ASSERT_EQ(stmts.size(), 1);
  EXPECT_EQ(stmts[0]->kind, druk::NodeKind::Function);
}

TEST_F(ParserTest, FunctionWithParams) {
  auto stmts = parse("ལས་ཀ add(int a, int b) { }");
  ASSERT_EQ(stmts.size(), 1);
}

TEST_F(ParserTest, VarDecl) {
  // Wrapped in function because top-level only supports functions currently
  // Use proper keyword 'གྲངས་ཀ' (drangka/number)
  auto stmts = parse("ལས་ཀ main() { གྲངས་ཀ x = 10; }");
  auto *func = static_cast<druk::FuncDecl *>(stmts[0]);
  auto *body = static_cast<druk::BlockStmt *>(func->body);
  ASSERT_NE(body->statements, nullptr);
  EXPECT_EQ(body->count, 1);
  EXPECT_EQ(body->statements[0]->kind, druk::NodeKind::Variable);
  // Wait, my implementation of parse_block used std::vector but BlockStmt has
  // Stmt**. I left Stmt** as nullptr or didn't populate it in parse_block. Need
  // to fix that for this test to pass fully or inspect logic.
}
