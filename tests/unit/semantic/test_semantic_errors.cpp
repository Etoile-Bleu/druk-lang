// test_semantic_errors.cpp — Semantic analysis detects errors in invalid programs
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;

class SemanticErrorsTest : public ::testing::Test
{
   protected:
    SemanticHelper sh;
};

// ─── Valid code returns true ──────────────────────────────────────────────────

TEST_F(SemanticErrorsTest, ValidCodeReturnsTrue)
{
    EXPECT_TRUE(sh.analyze("གྲངས་ x = ༥; བཀོད་ x;"));
}

TEST_F(SemanticErrorsTest, EmptyProgramIsValid)
{
    EXPECT_TRUE(sh.analyze(""));
}

// ─── Use before declaration ───────────────────────────────────────────────────

TEST_F(SemanticErrorsTest, UseUndeclaredVariableFails)
{
    // Reading 'y' which was never declared should fail semantic analysis
    bool ok = sh.analyze("བཀོད་ y;");
    EXPECT_FALSE(ok);
}

TEST_F(SemanticErrorsTest, UseVariableBeforeDeclarationFails)
{
    // x is used before it is declared in the same scope
    bool ok = sh.analyze("བཀོད་ x; གྲངས་ x = ༥;");
    EXPECT_FALSE(ok);
}

// ─── Assignments to undeclared variables ─────────────────────────────────────

TEST_F(SemanticErrorsTest, AssignToUndeclaredVariable)
{
    // Assigning to a variable that was never declared
    bool ok = sh.analyze("z = ༣;");
    EXPECT_FALSE(ok);
}

// ─── Function call to undeclared function ────────────────────────────────────

TEST_F(SemanticErrorsTest, CallUndeclaredFunction)
{
    bool ok = sh.analyze("foo();");
    EXPECT_FALSE(ok);
}

// ─── Error state on malformed source ─────────────────────────────────────────

TEST_F(SemanticErrorsTest, ParseErrorPreventsSemantic)
{
    // Malformed source: parser errors prevent semantic from running
    bool ok = sh.analyze("གྲངས་ x = ;");  // missing initializer
    EXPECT_FALSE(ok);
}
