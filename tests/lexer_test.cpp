#include "druk/lexer/lexer.hpp"
#include <gtest/gtest.h>
#include <vector>


class LexerTest : public ::testing::Test {
protected:
  druk::ArenaAllocator arena;
  druk::StringInterner interner{arena};
  druk::ErrorReporter errors;

  std::vector<druk::Token> tokenize(std::string_view source) {
    druk::Lexer lexer(source, arena, interner, errors);
    std::vector<druk::Token> tokens;
    while (true) {
      druk::Token token = lexer.next();
      if (token.kind == druk::TokenKind::EndOfFile)
        break;
      tokens.push_back(token);
    }
    return tokens;
  }
};

TEST_F(LexerTest, BasicTokens) {
  auto tokens = tokenize("(){},;");
  ASSERT_EQ(tokens.size(), 6);
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::LParen);
  EXPECT_EQ(tokens[5].kind, druk::TokenKind::Semicolon);
}

TEST_F(LexerTest, Operators) {
  auto tokens = tokenize("= == ! !=");
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::Equal);
  EXPECT_EQ(tokens[1].kind, druk::TokenKind::EqualEqual);
  EXPECT_EQ(tokens[2].kind, druk::TokenKind::Bang);
  EXPECT_EQ(tokens[3].kind, druk::TokenKind::BangEqual);
}

TEST_F(LexerTest, Identifiers) {
  auto tokens = tokenize("abc var1");
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::Identifier);
  EXPECT_EQ(tokens[1].kind, druk::TokenKind::Identifier);
}

TEST_F(LexerTest, Keywords) {
  // ལས་ཀ (Function)
  auto tokens = tokenize("ལས་ཀ");
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::KwFunction);

  // གལ་ཏེ (If)
  tokens = tokenize("གལ་ཏེ");
  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::KwIf);
}

TEST_F(LexerTest, Numbers) {
  auto tokens = tokenize("123 ༠༡༢");
  // 123 (ASCII), ༠༡༢ (Tibetan 012)
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::Number);
  EXPECT_EQ(tokens[1].kind, druk::TokenKind::Number);
}

TEST_F(LexerTest, Strings) {
  auto tokens = tokenize("\"hello\" \"world\"");
  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::String);
}

TEST_F(LexerTest, SkipComments) {
  auto tokens = tokenize("// comment\nTOKEN");
  ASSERT_EQ(tokens.size(), 1);
  // Should be Identifier "TOKEN"
  EXPECT_EQ(tokens[0].kind, druk::TokenKind::Identifier);
}
