// test_pipeline.cpp — Integration: Lex + Parse + Semantic chained together
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;
using namespace druk::parser::ast;

// ─── Full pipeline via SemanticHelper ────────────────────────────────────────

class PipelineTest : public ::testing::Test
{
   protected:
    SemanticHelper sh;
};

TEST_F(PipelineTest, SimpleVariableAndPrint)
{
    EXPECT_TRUE(sh.analyze("གྲངས་ x = ༡; བཀོད་ x;"));
}

TEST_F(PipelineTest, ArithmeticExpression)
{
    EXPECT_TRUE(sh.analyze("གྲངས་ r = ༩ + ༣ * ༢; བཀོད་ r;"));
}

TEST_F(PipelineTest, FunctionDeclarationAndCall)
{
    EXPECT_TRUE(
        sh.analyze("ལས་འགན་ add(གྲངས་ a, གྲངས་ b) { སླར་ལོག་ a + b; }"
                   "གྲངས་ result = add(༣, ༤);"
                   "བཀོད་ result;"));
}

TEST_F(PipelineTest, IfElseBranching)
{
    EXPECT_TRUE(
        sh.analyze("གྲངས་ x = ༧;"
                   "གལ་སྲིད་ (x > ༥) { བཀོད་ \"big\"; } མེད་ན་ { བཀོད་ \"small\"; }"));
}

TEST_F(PipelineTest, WhileLoopCountdown)
{
    EXPECT_TRUE(
        sh.analyze("གྲངས་ n = ༥;"
                   "ཡང་བསྐྱར་ (n > ༠) { n = n - ༡; }"
                   "བཀོད་ n;"));
}

TEST_F(PipelineTest, ForLoopWithBody)
{
    EXPECT_TRUE(sh.analyze("རེ་རེར་ (གྲངས་ i = ༠; i < ༣; i = i + ༡) { བཀོད་ i; }"));
}

TEST_F(PipelineTest, RecursiveFibonacciDeclaration)
{
    // Parser and semantic must accept valid recursive declarations
    EXPECT_TRUE(
        sh.analyze("ལས་འགན་ fib(གྲངས་ n) {"
                   "  གལ་སྲིད་ (n < ༢) { སླར་ལོག་ n; }"
                   "  སླར་ལོག་ fib(n - ༡) + fib(n - ༢);"
                   "}"));
}

// ─── Error propagation ────────────────────────────────────────────────────────

TEST_F(PipelineTest, ParseErrorBlocksSemantic)
{
    // Bad syntax → pipeline should fail
    EXPECT_FALSE(sh.analyze("གྲངས་ x = ;"));
}

TEST_F(PipelineTest, SemanticErrorOnUndeclared)
{
    EXPECT_FALSE(sh.analyze("བཀོད་ undeclared_var;"));
}

// ─── Multi-statement programs ─────────────────────────────────────────────────

TEST_F(PipelineTest, MultipleStatements)
{
    EXPECT_TRUE(
        sh.analyze("གྲངས་ a = ༡;"
                   "གྲངས་ b = ༢;"
                   "གྲངས་ c = a + b;"
                   "བཀོད་ c;"));
}

TEST_F(PipelineTest, FunctionWithLocalThenReturn)
{
    EXPECT_TRUE(
        sh.analyze("ལས་འགན་ compute(གྲངས་ x) {"
                   "  གྲངས་ doubled = x * ༢;"
                   "  སླར་ལོག་ doubled;"
                   "}"));
}
