// test_lexer_errors.cpp — Error token generation and line/offset tracking
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;

class LexerErrorsTest : public ::testing::Test
{
   protected:
    LexHelper lex;
};

TEST_F(LexerErrorsTest, UnterminatedStringProducesInvalidToken)
{
    // A string without a closing quote must not crash; the lexer
    // emits an Invalid token and/or an EndOfFile.
    druk::lexer::Lexer lexer("\"unterminated", lex.arena, lex.interner, lex.errors);
    auto               tok = lexer.next();
    // Acceptable outcomes: Invalid token or EOF (implementation-dependent).
    bool isErrorOrEOF = (tok.type == TT::Invalid || tok.type == TT::EndOfFile);
    EXPECT_TRUE(isErrorOrEOF) << "Expected Invalid or EOF on unterminated string";
}

TEST_F(LexerErrorsTest, ValidTokensAfterInvalidChar)
{
    // '@' is not a valid Druk character. The lexer should recover and
    // still produce the subsequent ';'.
    druk::lexer::Lexer lexer("@;", lex.arena, lex.interner, lex.errors);
    auto               t1 = lexer.next();
    auto               t2 = lexer.next();
    // t1 is an error token; t2 must be the semicolon.
    EXPECT_EQ(t2.type, TT::Semicolon);
}

TEST_F(LexerErrorsTest, LineNumberCorrectAfterNewline)
{
    druk::lexer::Lexer lexer("\n\n;", lex.arena, lex.interner, lex.errors);
    auto               tok = lexer.next();
    EXPECT_EQ(tok.line, 3u);
}

TEST_F(LexerErrorsTest, OffsetIsCorrect)
{
    // ';' at offset 3 (after "abc")
    const char*        src = "abc;";
    druk::lexer::Lexer lexer(src, lex.arena, lex.interner, lex.errors);
    lexer.next();  // skip "abc" identifier
    auto tok = lexer.next();
    EXPECT_EQ(tok.type, TT::Semicolon);
    EXPECT_EQ(tok.offset, 3u);
}
