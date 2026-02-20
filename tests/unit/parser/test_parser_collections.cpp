// test_parser_collections.cpp — Array and struct literals, index access, member access
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"

using namespace druk::test;
using namespace druk::parser::ast;

class ParserCollectionsTest : public ::testing::Test
{
   protected:
    ParseHelper ph;
};

// ─── Array literals ───────────────────────────────────────────────────────────

TEST_F(ParserCollectionsTest, EmptyArray)
{
    auto  stmts = ph.parse("[];");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* arr = dynamic_cast<ArrayLiteralExpr*>(es->expression);
    ASSERT_TRUE(arr);
    EXPECT_EQ(arr->count, 0u);
}

TEST_F(ParserCollectionsTest, ArrayThreeElements)
{
    auto  stmts = ph.parse("[1, 2, 3];");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* arr   = dynamic_cast<ArrayLiteralExpr*>(es->expression);
    ASSERT_TRUE(arr);
    EXPECT_EQ(arr->count, 3u);
}

TEST_F(ParserCollectionsTest, ArraySingleElement)
{
    auto  stmts = ph.parse("[42];");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* arr   = dynamic_cast<ArrayLiteralExpr*>(es->expression);
    ASSERT_TRUE(arr);
    EXPECT_EQ(arr->count, 1u);
}

// ─── Index access ─────────────────────────────────────────────────────────────

TEST_F(ParserCollectionsTest, IndexAccess)
{
    auto  stmts = ph.parse("arr[0];");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* idx = dynamic_cast<IndexExpr*>(es->expression);
    ASSERT_TRUE(idx);
    EXPECT_TRUE(idx->array != nullptr);
    EXPECT_TRUE(idx->index != nullptr);
}

TEST_F(ParserCollectionsTest, NestedIndexAccess)
{
    auto  stmts = ph.parse("arr[a[0]];");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* outer = dynamic_cast<IndexExpr*>(es->expression);
    ASSERT_TRUE(outer);
    auto* inner = dynamic_cast<IndexExpr*>(outer->index);
    ASSERT_TRUE(inner);
}

// ─── Struct literals ──────────────────────────────────────────────────────────

TEST_F(ParserCollectionsTest, StructLiteralTwoFields)
{
    auto  stmts = ph.parse("({x: 1, y: 2});");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* group = dynamic_cast<GroupingExpr*>(es->expression);
    ASSERT_TRUE(group);
    auto* s = dynamic_cast<StructLiteralExpr*>(group->expression);
    ASSERT_TRUE(s);
    EXPECT_EQ(s->fieldCount, 2u);
}

TEST_F(ParserCollectionsTest, StructLiteralOneField)
{
    auto  stmts = ph.parse("({name: \"druk\"});");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* group = dynamic_cast<GroupingExpr*>(es->expression);
    ASSERT_TRUE(group);
    auto* s = dynamic_cast<StructLiteralExpr*>(group->expression);
    ASSERT_TRUE(s);
    EXPECT_EQ(s->fieldCount, 1u);
}

// ─── Member access ────────────────────────────────────────────────────────────

TEST_F(ParserCollectionsTest, MemberAccess)
{
    auto  stmts = ph.parse("point.x;");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* mem = dynamic_cast<MemberAccessExpr*>(es->expression);
    ASSERT_TRUE(mem);
    EXPECT_TRUE(mem->object != nullptr);
}

TEST_F(ParserCollectionsTest, NoErrorsOnArrayAndStruct)
{
    ph.parse("[1, 2]; ({a: 3});");
    EXPECT_TRUE(ph.noErrors());
}
