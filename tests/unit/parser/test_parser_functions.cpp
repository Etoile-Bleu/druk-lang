// test_parser_functions.cpp — Function declarations, calls, recursion
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;
using namespace druk::parser::ast;

class ParserFunctionsTest : public ::testing::Test
{
   protected:
    ParseHelper ph;
};

// ─── Function declarations ────────────────────────────────────────────────────

TEST_F(ParserFunctionsTest, NoParamFunction)
{
    auto stmts = ph.parse("ལས་འགན་ greet() { བཀོད་ \"hello\"; }");
    ASSERT_EQ(stmts.size(), 1u);
    auto* func = dynamic_cast<FuncDecl*>(stmts[0]);
    ASSERT_TRUE(func);
    EXPECT_EQ(func->paramCount, 0u);
}

TEST_F(ParserFunctionsTest, OneParamFunction)
{
    auto  stmts = ph.parse("ལས་འགན་ inc(གྲངས་ n) { སླར་ལོག་ n + ༡; }");
    auto* func  = dynamic_cast<FuncDecl*>(stmts[0]);
    ASSERT_TRUE(func);
    EXPECT_EQ(func->paramCount, 1u);
}

TEST_F(ParserFunctionsTest, TwoParamFunction)
{
    auto  stmts = ph.parse("ལས་འགན་ add(གྲངས་ a, གྲངས་ b) { སླར་ལོག་ a + b; }");
    auto* func  = dynamic_cast<FuncDecl*>(stmts[0]);
    ASSERT_TRUE(func);
    EXPECT_EQ(func->paramCount, 2u);
}

TEST_F(ParserFunctionsTest, FunctionBodyIsBlock)
{
    auto  stmts = ph.parse("ལས་འགན་ f() { བཀོད་ ༠; }");
    auto* func  = dynamic_cast<FuncDecl*>(stmts[0]);
    ASSERT_TRUE(func);
    EXPECT_TRUE(dynamic_cast<BlockStmt*>(func->body));
}

TEST_F(ParserFunctionsTest, FunctionWithReturnStatement)
{
    auto  stmts = ph.parse("ལས་འགན་ f(གྲངས་ x) { སླར་ལོག་ x; }");
    auto* func  = dynamic_cast<FuncDecl*>(stmts[0]);
    ASSERT_TRUE(func);
    auto* block = dynamic_cast<BlockStmt*>(func->body);
    ASSERT_TRUE(block);
    ASSERT_EQ(block->count, 1u);
    EXPECT_TRUE(dynamic_cast<ReturnStmt*>(block->statements[0]));
}

// ─── Call expressions ─────────────────────────────────────────────────────────

TEST_F(ParserFunctionsTest, CallNoArgs)
{
    auto  stmts = ph.parse("f();");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    ASSERT_TRUE(es);
    auto* call = dynamic_cast<CallExpr*>(es->expression);
    ASSERT_TRUE(call);
    EXPECT_EQ(call->argCount, 0u);
}

TEST_F(ParserFunctionsTest, CallOneArg)
{
    auto  stmts = ph.parse("f(42);");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* call  = dynamic_cast<CallExpr*>(es->expression);
    ASSERT_TRUE(call);
    EXPECT_EQ(call->argCount, 1u);
}

TEST_F(ParserFunctionsTest, CallTwoArgs)
{
    auto  stmts = ph.parse("add(1, 2);");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* call  = dynamic_cast<CallExpr*>(es->expression);
    ASSERT_TRUE(call);
    EXPECT_EQ(call->argCount, 2u);
}

TEST_F(ParserFunctionsTest, NestedCallExpr)
{
    // f(g(1))
    auto  stmts = ph.parse("f(g(1));");
    auto* es    = dynamic_cast<ExpressionStmt*>(stmts[0]);
    auto* outer = dynamic_cast<CallExpr*>(es->expression);
    ASSERT_TRUE(outer);
    ASSERT_EQ(outer->argCount, 1u);
    auto* inner = dynamic_cast<CallExpr*>(reinterpret_cast<Expr*>(outer->args[0]));
    ASSERT_TRUE(inner);
}

TEST_F(ParserFunctionsTest, NoErrorsOnValidFunction)
{
    ph.parse("ལས་འགན་ square(གྲངས་ n) { སླར་ལོག་ n * n; }");
    EXPECT_TRUE(ph.noErrors());
}
