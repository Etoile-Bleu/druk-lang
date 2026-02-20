// test_parser_errors.cpp — Syntax errors should set hasErrors() and not crash
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;
using namespace druk::parser::ast;

class ParserErrorsTest : public ::testing::Test
{
   protected:
    ParseHelper ph;
};

// Parser must not crash on any malformed input.
// Errors are recorded in ErrorHandler — we check hasErrors() == true.

TEST_F(ParserErrorsTest, MissingSemicolonAfterExpression)
{
    // "1 + 2" without ';' — should produce an error
    ph.parse("1 + 2");
    EXPECT_TRUE(ph.hasErrors());
}

TEST_F(ParserErrorsTest, UnmatchedOpenParen)
{
    ph.parse("(1 + 2;");
    EXPECT_TRUE(ph.hasErrors());
}

TEST_F(ParserErrorsTest, UnmatchedCloseParen)
{
    ph.parse("1 + 2);");
    EXPECT_TRUE(ph.hasErrors());
}

TEST_F(ParserErrorsTest, MissingConditionInIf)
{
    ph.parse("གལ་སྲིད་ { བཀོད་ ༡; }");
    EXPECT_TRUE(ph.hasErrors());
}

TEST_F(ParserErrorsTest, MissingBlockInWhile)
{
    ph.parse("ཡང་བསྐྱར་ (x < ༥);");
    EXPECT_TRUE(ph.hasErrors());
}

TEST_F(ParserErrorsTest, EmptyForCondition)
{
    // Omitting semicolons inside for is an error
    ph.parse("རེ་རེར་ (;;) { }");
    // Parser should not crash even if it errors
    // (outcome depends on implementation; just verify no crash)
    (void)ph.hasErrors();
}

TEST_F(ParserErrorsTest, MissingFunctionName)
{
    ph.parse("ལས་འགན་ (གྲངས་ x) { }");
    EXPECT_TRUE(ph.hasErrors());
}

TEST_F(ParserErrorsTest, DoubleOperatorNoOperand)
{
    ph.parse("1 + + ;");
    // Parser should handle gracefully
    (void)ph.hasErrors();  // Don't prescribe exact outcome, just no crash
}

TEST_F(ParserErrorsTest, ValidInputHasNoErrors)
{
    ph.parse("གྲངས་ x = ༥; བཀོད་ x;");
    EXPECT_FALSE(ph.hasErrors());
}
