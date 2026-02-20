// test_lexer_keywords.cpp — All Dzongkha keywords map to the correct TokenType
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;

class LexerKeywordsTest : public ::testing::Test
{
   protected:
    LexHelper lex;
};

// ─── Current keyword table (from identifier.cpp) ─────────────────────────────
// ལས་འགན་ → KwFunction     གྲངས་     → KwNumber
// ཡིག་འབྲུ་  → KwString      བདེན་རྫུན་ → KwBoolean
// གལ་སྲིད་  → KwIf          མེད་ན་    → KwElse
// ཡང་བསྐྱར་ → KwWhile/Loop  རེ་རེར་   → KwFor
// རིམ་པ་   → KwLoop        སླར་ལོག་   → KwReturn
// བཀོད་    → KwPrint       བདེན་པ་   → KwTrue
// རྫུན་མ་   → KwFalse

TEST_F(LexerKeywordsTest, KwFunction)
{
    EXPECT_EQ(lex.first("ལས་འགན་").type, TT::KwFunction);
}
TEST_F(LexerKeywordsTest, KwNumber)
{
    EXPECT_EQ(lex.first("གྲངས་").type, TT::KwNumber);
}
TEST_F(LexerKeywordsTest, KwString)
{
    EXPECT_EQ(lex.first("ཡིག་འབྲུ་").type, TT::KwString);
}
TEST_F(LexerKeywordsTest, KwBoolean)
{
    EXPECT_EQ(lex.first("བདེན་རྫུན་").type, TT::KwBoolean);
}
TEST_F(LexerKeywordsTest, KwIf)
{
    EXPECT_EQ(lex.first("གལ་སྲིད་").type, TT::KwIf);
}
TEST_F(LexerKeywordsTest, KwElse)
{
    EXPECT_EQ(lex.first("མེད་ན་").type, TT::KwElse);
}
TEST_F(LexerKeywordsTest, KwWhile)
{
    EXPECT_EQ(lex.first("ཡང་བསྐྱར་").type, TT::KwWhile);
}
TEST_F(LexerKeywordsTest, KwFor)
{
    EXPECT_EQ(lex.first("རེ་རེར་").type, TT::KwFor);
}
TEST_F(LexerKeywordsTest, KwLoopLegacy)
{
    // རིམ་པ་ is the legacy loop keyword still supported
    EXPECT_EQ(lex.first("རིམ་པ་").type, TT::KwLoop);
}
TEST_F(LexerKeywordsTest, KwReturn)
{
    EXPECT_EQ(lex.first("སླར་ལོག་").type, TT::KwReturn);
}
TEST_F(LexerKeywordsTest, KwPrint)
{
    EXPECT_EQ(lex.first("བཀོད་").type, TT::KwPrint);
}
TEST_F(LexerKeywordsTest, KwTrue)
{
    EXPECT_EQ(lex.first("བདེན་པ་").type, TT::KwTrue);
}
TEST_F(LexerKeywordsTest, KwFalse)
{
    EXPECT_EQ(lex.first("རྫུན་མ་").type, TT::KwFalse);
}

// ─── Non-keyword identifiers ──────────────────────────────────────────────────

TEST_F(LexerKeywordsTest, AsciiIdentifierIsNotKeyword)
{
    EXPECT_EQ(lex.first("foo").type, TT::Identifier);
}

TEST_F(LexerKeywordsTest, TibetanIdentifierIsNotKeyword)
{
    // Any Tibetan string not in the keyword table → Identifier
    EXPECT_EQ(lex.first("བོད་").type, TT::Identifier);
}

TEST_F(LexerKeywordsTest, PartialKeywordIsIdentifier)
{
    // "གྲང" is not "གྲངས་" so it is an identifier
    EXPECT_EQ(lex.first("གྲང").type, TT::Identifier);
}

TEST_F(LexerKeywordsTest, AllKeywordsInSequence)
{
    const char* src =
        "ལས་འགན་ གྲངས་ ཡིག་འབྲུ་ བདེན་རྫུན་ "
        "གལ་སྲིད་ མེད་ན་ ཡང་བསྐྱར་ རེ་རེར་ རིམ་པ་ "
        "སླར་ལོག་ བཀོད་ བདེན་པ་ རྫུན་མ་";
    auto toks = lex.tokenize(src);
    ASSERT_EQ(toks.size(), 13u);
    EXPECT_EQ(toks[0].type, TT::KwFunction);
    EXPECT_EQ(toks[1].type, TT::KwNumber);
    EXPECT_EQ(toks[2].type, TT::KwString);
    EXPECT_EQ(toks[3].type, TT::KwBoolean);
    EXPECT_EQ(toks[4].type, TT::KwIf);
    EXPECT_EQ(toks[5].type, TT::KwElse);
    EXPECT_EQ(toks[6].type, TT::KwWhile);
    EXPECT_EQ(toks[7].type, TT::KwFor);
    EXPECT_EQ(toks[8].type, TT::KwLoop);
    EXPECT_EQ(toks[9].type, TT::KwReturn);
    EXPECT_EQ(toks[10].type, TT::KwPrint);
    EXPECT_EQ(toks[11].type, TT::KwTrue);
    EXPECT_EQ(toks[12].type, TT::KwFalse);
}
