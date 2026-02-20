// test_parser_expressions.cpp — Arithmetic, comparison, and logical expressions
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;
using namespace druk::parser::ast;

class ParserExpressionsTest : public ::testing::Test
{
   protected:
    ParseHelper ph;
};

// ─── Literals ─────────────────────────────────────────────────────────────────

TEST_F(ParserExpressionsTest, ParsesIntegerLiteral)
{
    auto stmts = ph.parse("123;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* es = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression);
    ASSERT_TRUE(lit);
    EXPECT_EQ(lit->literalValue.asInt(), 123);
}

TEST_F(ParserExpressionsTest, ParsesTibetanIntegerLiteral)
{
    auto stmts = ph.parse("༥;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* es = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression);
    ASSERT_TRUE(lit);
    EXPECT_EQ(lit->literalValue.asInt(), 5);
}

TEST_F(ParserExpressionsTest, ParsesTrueLiteral)
{
    auto stmts = ph.parse("བདེན་པ་;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* es = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression);
    ASSERT_TRUE(lit);
    EXPECT_TRUE(lit->literalValue.asBool());
}

TEST_F(ParserExpressionsTest, ParsesFalseLiteral)
{
    auto stmts = ph.parse("རྫུན་མ་;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* es = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* lit = dynamic_cast<LiteralExpr*>(es->expression);
    ASSERT_TRUE(lit);
    EXPECT_FALSE(lit->literalValue.asBool());
}

// ─── Binary expressions ───────────────────────────────────────────────────────

TEST_F(ParserExpressionsTest, ParsesAddition)
{
    auto  stmts = ph.parse("1 + 2;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* bin   = dynamic_cast<BinaryExpr*>(es->expression);
    ASSERT_TRUE(bin);
    EXPECT_EQ(bin->token.type, TT::Plus);
}

TEST_F(ParserExpressionsTest, ParsesMultiplication)
{
    auto  stmts = ph.parse("3 * 4;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* bin   = dynamic_cast<BinaryExpr*>(es->expression);
    ASSERT_TRUE(bin);
    EXPECT_EQ(bin->token.type, TT::Star);
}

TEST_F(ParserExpressionsTest, PrecedenceMulBeforeAdd)
{
    // 1 + 2 * 3  →  BinaryExpr(+, 1, BinaryExpr(*, 2, 3))
    auto  stmts = ph.parse("1 + 2 * 3;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* root  = dynamic_cast<BinaryExpr*>(es->expression);
    ASSERT_TRUE(root);
    EXPECT_EQ(root->token.type, TT::Plus);
    auto* rhs = dynamic_cast<BinaryExpr*>(root->right);
    ASSERT_TRUE(rhs);
    EXPECT_EQ(rhs->token.type, TT::Star);
}

TEST_F(ParserExpressionsTest, GroupingOverridesPrecedence)
{
    // (1 + 2) * 3  →  BinaryExpr(*, Grouping(+), 3)
    auto  stmts = ph.parse("(1 + 2) * 3;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* root  = dynamic_cast<BinaryExpr*>(es->expression);
    ASSERT_TRUE(root);
    EXPECT_EQ(root->token.type, TT::Star);
    auto* lhs = dynamic_cast<GroupingExpr*>(root->left);
    ASSERT_TRUE(lhs);
}

// ─── Comparison & equality ────────────────────────────────────────────────────

TEST_F(ParserExpressionsTest, ParsesLessThan)
{
    auto  stmts = ph.parse("a < b;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* bin   = dynamic_cast<BinaryExpr*>(es->expression);
    ASSERT_TRUE(bin);
    EXPECT_EQ(bin->token.type, TT::Less);
}

TEST_F(ParserExpressionsTest, ParsesEqualEqual)
{
    auto  stmts = ph.parse("a == b;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* bin   = dynamic_cast<BinaryExpr*>(es->expression);
    ASSERT_TRUE(bin);
    EXPECT_EQ(bin->token.type, TT::EqualEqual);
}

TEST_F(ParserExpressionsTest, ParsesBangEqual)
{
    auto  stmts = ph.parse("a != b;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* bin   = dynamic_cast<BinaryExpr*>(es->expression);
    ASSERT_TRUE(bin);
    EXPECT_EQ(bin->token.type, TT::BangEqual);
}

// ─── Variable expressions ─────────────────────────────────────────────────────

TEST_F(ParserExpressionsTest, ParsesVariableExpression)
{
    auto  stmts = ph.parse("myVar;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* var   = dynamic_cast<VariableExpr*>(es->expression);
    ASSERT_TRUE(var);
}

// ─── Assignment ───────────────────────────────────────────────────────────────

TEST_F(ParserExpressionsTest, ParsesAssignment)
{
    // Must declare before use for semantic, but parser doesn't care
    auto  stmts  = ph.parse("x = 42;");
    auto* es     = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* assign = dynamic_cast<AssignmentExpr*>(es->expression);
    ASSERT_TRUE(assign);
}

TEST_F(ParserExpressionsTest, NoParseErrors)
{
    ph.parse("1 + 2 * 3;");
    EXPECT_TRUE(ph.noErrors());
}
