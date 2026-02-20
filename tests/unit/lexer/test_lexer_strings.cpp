// test_lexer_strings.cpp — String literal scanning
#include <gtest/gtest.h>

#include <string>

#include "helpers/test_helpers.h"


using namespace druk::test;

class LexerStringsTest : public ::testing::Test
{
   protected:
    LexHelper lex;

    std::string textOf(std::string_view src)
    {
        druk::lexer::Lexer lexer(src, lex.arena, lex.interner, lex.errors);
        auto               tok = lexer.next();
        return std::string(tok.text(src));
    }
};

TEST_F(LexerStringsTest, SimpleAsciiString)
{
    auto tok = lex.first("\"hello\"");
    EXPECT_EQ(tok.type, TT::String);
}
TEST_F(LexerStringsTest, StringTextIncludesQuotes)
{
    // Token text includes the surrounding quotes
    EXPECT_EQ(textOf("\"hello\""), "\"hello\"");
}
TEST_F(LexerStringsTest, EmptyString)
{
    auto tok = lex.first("\"\"");
    EXPECT_EQ(tok.type, TT::String);
    EXPECT_EQ(textOf("\"\""), "\"\"");
}
TEST_F(LexerStringsTest, StringWithSpaces)
{
    auto tok = lex.first("\"hello world\"");
    EXPECT_EQ(tok.type, TT::String);
}
TEST_F(LexerStringsTest, DzongkhaString)
{
    auto tok = lex.first("\"བཀྲ་ཤིས་བདེ་ལེགས།\"");
    EXPECT_EQ(tok.type, TT::String);
}
TEST_F(LexerStringsTest, TwoAdjacentStrings)
{
    auto toks = lex.tokenize("\"a\" \"b\"");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::String);
    EXPECT_EQ(toks[1].type, TT::String);
}
TEST_F(LexerStringsTest, StringFollowedBySemicolon)
{
    auto toks = lex.tokenize("\"ok\";");
    ASSERT_EQ(toks.size(), 2u);
    EXPECT_EQ(toks[0].type, TT::String);
    EXPECT_EQ(toks[1].type, TT::Semicolon);
}
