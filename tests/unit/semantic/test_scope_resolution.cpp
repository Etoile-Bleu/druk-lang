// test_scope_resolution.cpp — Variable scoping and shadowing
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"

using namespace druk::test;

class ScopeResolutionTest : public ::testing::Test
{
   protected:
    SemanticHelper sh;
};

// ─── Basic scope ──────────────────────────────────────────────────────────────

TEST_F(ScopeResolutionTest, VariableUsedInSameScope)
{
    EXPECT_TRUE(sh.analyze("གྲངས་ x = ༥; བཀོད་ x;"));
}

TEST_F(ScopeResolutionTest, TwoVariablesInSameScope)
{
    EXPECT_TRUE(sh.analyze("གྲངས་ a = ༡; གྲངས་ b = ༢; བཀོད་ a + b;"));
}

// ─── Nested scopes (if / while bodies) ───────────────────────────────────────

TEST_F(ScopeResolutionTest, OuterVariableAccessibleInsideIf)
{
    EXPECT_TRUE(
        sh.analyze("གྲངས་ x = ༡༠;"
                   "གལ་སྲིད་ (x < ༢༠) { བཀོད་ x; }"));
}

TEST_F(ScopeResolutionTest, OuterVariableAccessibleInsideWhile)
{
    EXPECT_TRUE(
        sh.analyze("གྲངས་ n = ༠;"
                   "ཡང་བསྐྱར་ (n < ༥) { n = n + ༡; }"));
}

TEST_F(ScopeResolutionTest, ForLoopInitVariableUsedInBody)
{
    EXPECT_TRUE(sh.analyze("རེ་རེར་ (གྲངས་ i = ༠; i < ༣; i = i + ༡) { བཀོད་ i; }"));
}

// ─── Functions have their own scope ──────────────────────────────────────────

TEST_F(ScopeResolutionTest, FunctionParamUsedInBody)
{
    EXPECT_TRUE(sh.analyze("ལས་འགན་ double(གྲངས་ n) { སླར་ལོག་ n * ༢; }"));
}

TEST_F(ScopeResolutionTest, TwoFunctionsWithSameParamName)
{
    EXPECT_TRUE(
        sh.analyze("ལས་འགན་ f(གྲངས་ x) { སླར་ལོག་ x; }"
                   "ལས་འགན་ g(གྲངས་ x) { སླར་ལོག་ x * ༢; }"));
}
