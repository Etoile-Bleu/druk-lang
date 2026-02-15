#include "druk/lexer/token.hpp"
#include <gtest/gtest.h>


TEST(TokenTest, Size) { EXPECT_EQ(sizeof(druk::Token), 16); }

TEST(TokenTest, Layout) {
  druk::Token t;
  t.kind = druk::TokenKind::Identifier;
  t.offset = 0;
  t.length = 5;
  t.line = 1;

  EXPECT_EQ(t.kind, druk::TokenKind::Identifier);
  EXPECT_EQ(t.length, 5);
}

TEST(TokenTest, TextMethod) {
  std::string_view source = "hello world";
  druk::Token t;
  t.offset = 0;
  t.length = 5;
  EXPECT_EQ(t.text(source), "hello");
}
