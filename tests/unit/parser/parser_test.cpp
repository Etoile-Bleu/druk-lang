#include "druk/parser/core/parser.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/util/arena_allocator.hpp"
#include "druk/util/error_handler.hpp"
#include <gtest/gtest.h>

using namespace druk::parser;
using namespace druk::util;
using namespace druk::lexer;

class ParserTest : public ::testing::Test {
protected:
  ArenaAllocator arena;
  StringInterner interner{arena};
  ErrorHandler errors;

  Parser createParser(std::string_view source) {
    return Parser(source, arena, interner, errors);
  }
};

TEST_F(ParserTest, ParsesLiteralExpressions) {
  auto parser = createParser("123;");
  auto stmts = parser.parse();
  
  ASSERT_EQ(stmts.size(), 1);
  auto* stmt = dynamic_cast<ast::ExpressionStmt*>(stmts[0]);
  ASSERT_TRUE(stmt != nullptr);
  
  auto* expr = dynamic_cast<ast::LiteralExpr*>(stmt->expression);
  ASSERT_TRUE(expr != nullptr);
  EXPECT_EQ(expr->literalValue.asInt(), 123);
}

TEST_F(ParserTest, ParsesBinaryExpressions) {
  auto parser = createParser("1 + 2 * 3;");
  auto stmts = parser.parse();
  
  ASSERT_EQ(stmts.size(), 1);
  auto* stmt = dynamic_cast<ast::ExpressionStmt*>(stmts[0]);
  ASSERT_TRUE(stmt != nullptr);
  
  auto* expr = dynamic_cast<ast::BinaryExpr*>(stmt->expression);
  ASSERT_TRUE(expr != nullptr);
  EXPECT_EQ(expr->token.type, TokenType::Plus);
}

TEST_F(ParserTest, ParsesVariableDeclaration) {
  auto parser = createParser("གྲངས་ཀ་ x = 10;");
  auto stmts = parser.parse();
  
  ASSERT_EQ(stmts.size(), 1);
  auto* decl = dynamic_cast<ast::VarDecl*>(stmts[0]);
  ASSERT_TRUE(decl != nullptr);
  
  auto* init = dynamic_cast<ast::LiteralExpr*>(decl->initializer);
  ASSERT_TRUE(init != nullptr);
  EXPECT_EQ(init->literalValue.asInt(), 10);
}

TEST_F(ParserTest, ParsesFunctionDeclaration) {
  auto parser = createParser("ལས་ཀ་ test_func(གྲངས་ཀ་ a, གྲངས་ཀ་ b) { འབྲི་ a + b; }");
  auto stmts = parser.parse();
  
  ASSERT_EQ(stmts.size(), 1);
  auto* func = dynamic_cast<ast::FuncDecl*>(stmts[0]);
  ASSERT_TRUE(func != nullptr);
  EXPECT_EQ(func->paramCount, 2);
  
  auto* body = dynamic_cast<ast::BlockStmt*>(func->body);
  ASSERT_TRUE(body != nullptr);
  EXPECT_EQ(body->count, 1);
}

TEST_F(ParserTest, ParsesIfStatement) {
  auto parser = createParser("གལ་ཏེ་ (1 < 2) { འབྲི་ 1; } ཡང་ན་ { འབྲི་ 0; }");
  auto stmts = parser.parse();
  
  ASSERT_EQ(stmts.size(), 1);
  auto* stmt = dynamic_cast<ast::IfStmt*>(stmts[0]);
  ASSERT_TRUE(stmt != nullptr);
  ASSERT_TRUE(stmt->condition != nullptr);
  ASSERT_TRUE(stmt->thenBranch != nullptr);
  ASSERT_TRUE(stmt->elseBranch != nullptr);
}
