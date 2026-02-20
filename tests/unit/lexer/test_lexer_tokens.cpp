// test_lexer_tokens.cpp — Tests for all punctuation and operator tokens
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;

class LexerTokensTest : public ::testing::Test
{
   protected:
    LexHelper lex;
};

// ─── Punctuation ──────────────────────────────────────────────────────────────

TEST_F(LexerTokensTest, Parens)
{
    auto toks = lex.tokenize("()");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::LParen);
    EXPECT_EQ(toks[1].type, TT::RParen);
}

TEST_F(LexerTokensTest, Braces)
{
    auto toks = lex.tokenize("{}");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::LBrace);
    EXPECT_EQ(toks[1].type, TT::RBrace);
}

TEST_F(LexerTokensTest, Brackets)
{
    auto toks = lex.tokenize("[]");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::LBracket);
    EXPECT_EQ(toks[1].type, TT::RBracket);
}

TEST_F(LexerTokensTest, Comma)
{
    auto toks = lex.tokenize(",");
    ASSERT_EQ(toks.size(), 1u);
    EXPECT_EQ(toks[0].type, TT::Comma);
}

TEST_F(LexerTokensTest, Semicolon)
{
    auto toks = lex.tokenize(";");
    ASSERT_EQ(toks.size(), 1u);
    EXPECT_EQ(toks[0].type, TT::Semicolon);
}

TEST_F(LexerTokensTest, Colon)
{
    auto toks = lex.tokenize(":");
    ASSERT_EQ(toks.size(), 1u);
    EXPECT_EQ(toks[0].type, TT::Colon);
}

TEST_F(LexerTokensTest, Dot)
{
    auto toks = lex.tokenize(".");
    ASSERT_EQ(toks.size(), 1u);
    EXPECT_EQ(toks[0].type, TT::Dot);
}

// ─── Arithmetic operators ─────────────────────────────────────────────────────

TEST_F(LexerTokensTest, ArithmeticOperators)
{
    auto toks = lex.tokenize("+ - * /");
    ASSERT_EQ(toks.size(), 4u);
    EXPECT_EQ(toks[0].type, TT::Plus);
    EXPECT_EQ(toks[1].type, TT::Minus);
    EXPECT_EQ(toks[2].type, TT::Star);
    EXPECT_EQ(toks[3].type, TT::Slash);
}

// ─── Comparison operators ─────────────────────────────────────────────────────

TEST_F(LexerTokensTest, ComparisonOperators)
{
    auto toks = lex.tokenize("< <= > >=");
    ASSERT_EQ(toks.size(), 4u);
    EXPECT_EQ(toks[0].type, TT::Less);
    EXPECT_EQ(toks[1].type, TT::LessEqual);
    EXPECT_EQ(toks[2].type, TT::Greater);
    EXPECT_EQ(toks[3].type, TT::GreaterEqual);
}

// ─── Equality operators ───────────────────────────────────────────────────────

TEST_F(LexerTokensTest, EqualityOperators)
{
    auto toks = lex.tokenize("= == != !");
    ASSERT_EQ(toks.size(), 4u);
    EXPECT_EQ(toks[0].type, TT::Equal);
    EXPECT_EQ(toks[1].type, TT::EqualEqual);
    EXPECT_EQ(toks[2].type, TT::BangEqual);
    EXPECT_EQ(toks[3].type, TT::Bang);
}

// ─── Logical operators ────────────────────────────────────────────────────────

TEST_F(LexerTokensTest, LogicalOperators)
{
    auto toks = lex.tokenize("&& ||");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::And);
    EXPECT_EQ(toks[1].type, TT::Or);
}

// ─── Whitespace and comments ──────────────────────────────────────────────────

TEST_F(LexerTokensTest, WhitespaceSkipped)
{
    auto toks = lex.tokenize("  \t\n  ;  \n  ;  ");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::Semicolon);
    EXPECT_EQ(toks[1].type, TT::Semicolon);
}

TEST_F(LexerTokensTest, LineCommentSkipped)
{
    // Tokens before and after a line comment
    auto toks = lex.tokenize("; // this is a comment\n;");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::Semicolon);
    EXPECT_EQ(toks[1].type, TT::Semicolon);
}

TEST_F(LexerTokensTest, EmptySourceProducesNoTokens)
{
    auto toks = lex.tokenize("");
    EXPECT_TRUE(toks.empty());
}

TEST_F(LexerTokensTest, AllPunctuationInSequence)
{
    auto toks = lex.tokenize("(){},;:.");
    ASSERT_EQ(toks.size(), 8u);
    EXPECT_EQ(toks[0].type, TT::LParen);
    EXPECT_EQ(toks[1].type, TT::RParen);
    EXPECT_EQ(toks[2].type, TT::LBrace);
    EXPECT_EQ(toks[3].type, TT::RBrace);
    EXPECT_EQ(toks[4].type, TT::Comma);
    EXPECT_EQ(toks[5].type, TT::Semicolon);
    EXPECT_EQ(toks[6].type, TT::Colon);
    EXPECT_EQ(toks[7].type, TT::Dot);
}

// ─── Line tracking ────────────────────────────────────────────────────────────

TEST_F(LexerTokensTest, LineNumberIncreasesOnNewline)
{
    // Second ';' should be on line 2
    druk::lexer::Lexer lexer(";\n;", lex.arena, lex.interner, lex.errors);
    auto               t1 = lexer.next();
    auto               t2 = lexer.next();
    EXPECT_EQ(t1.line, 1u);
    EXPECT_EQ(t2.line, 2u);
}
