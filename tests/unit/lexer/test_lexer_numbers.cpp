// test_lexer_numbers.cpp — ASCII and Tibetan numeral scanning
#include <gtest/gtest.h>

#include <string>

#include "helpers/test_helpers.h"


using namespace druk::test;

class LexerNumbersTest : public ::testing::Test
{
   protected:
    LexHelper lex;

    // Helper: tokenize and return the text of the first token.
    std::string firstText(std::string_view src)
    {
        druk::lexer::Lexer lexer(src, lex.arena, lex.interner, lex.errors);
        auto               tok = lexer.next();
        return std::string(tok.text(src));
    }
};

// ─── ASCII integers ───────────────────────────────────────────────────────────

TEST_F(LexerNumbersTest, SingleDigitAscii)
{
    EXPECT_EQ(lex.first("0").type, TT::Number);
    EXPECT_EQ(firstText("0"), "0");
}
TEST_F(LexerNumbersTest, MultiDigitAscii)
{
    EXPECT_EQ(lex.first("123").type, TT::Number);
    EXPECT_EQ(firstText("123"), "123");
}
TEST_F(LexerNumbersTest, LargeAsciiNumber)
{
    EXPECT_EQ(lex.first("9999999").type, TT::Number);
}

// ─── Tibetan digits (U+0F20–U+0F29) ──────────────────────────────────────────

TEST_F(LexerNumbersTest, SingleTibetanZero)
{
    EXPECT_EQ(lex.first("༠").type, TT::Number);
    EXPECT_EQ(firstText("༠"), "༠");
}
TEST_F(LexerNumbersTest, SingleTibetanNine)
{
    EXPECT_EQ(lex.first("༩").type, TT::Number);
}
TEST_F(LexerNumbersTest, MultiTibetanDigits)
{
    auto toks = lex.tokenize("༠༡༢༣༤༥༦༧༨༩");
    ASSERT_EQ(toks.size(), 1u);
    EXPECT_EQ(toks[0].type, TT::Number);
}
TEST_F(LexerNumbersTest, TibetanNumberFollowedBySemicolon)
{
    auto toks = lex.tokenize("༡༢༣;");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::Number);
    EXPECT_EQ(toks[1].type, TT::Semicolon);
}

// ─── Multiple number tokens ───────────────────────────────────────────────────

TEST_F(LexerNumbersTest, TwoAsciiNumbers)
{
    auto toks = lex.tokenize("1 2");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::Number);
    EXPECT_EQ(toks[1].type, TT::Number);
}
TEST_F(LexerNumbersTest, MixedAsciiAndTibetan)
{
    auto toks = lex.tokenize("42 ༤༢");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::Number);
    EXPECT_EQ(toks[1].type, TT::Number);
}
