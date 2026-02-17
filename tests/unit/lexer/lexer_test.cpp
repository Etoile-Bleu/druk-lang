#include "druk/lexer/lexer.hpp"
#include "druk/lexer/token.hpp"
#include "druk/util/arena_allocator.hpp"
#include "druk/lexer/unicode.hpp"
#include "druk/util/error_handler.hpp"
#include <gtest/gtest.h>

using namespace druk::lexer;
using namespace druk::util;

class LexerTest : public ::testing::Test {
protected:
  ArenaAllocator arena;
  StringInterner interner{arena};
  ErrorHandler errors;

  Lexer createLexer(std::string_view source) {
    return Lexer(source, arena, interner, errors);
  }
};

TEST_F(LexerTest, ScansBasicTokens) {
  auto lexer = createLexer("( ) { } [ ] , : . - + ; * / ! != = == < <= > >= && ||");
  
  auto checkToken = [&](TokenType expectedType) {
    auto token = lexer.next();
    EXPECT_EQ(token.type, expectedType);
  };

  checkToken(TokenType::LParen);
  checkToken(TokenType::RParen);
  checkToken(TokenType::LBrace);
  checkToken(TokenType::RBrace);
  checkToken(TokenType::LBracket);
  checkToken(TokenType::RBracket);
  checkToken(TokenType::Comma);
  checkToken(TokenType::Colon);
  checkToken(TokenType::Dot);
  checkToken(TokenType::Minus);
  checkToken(TokenType::Plus);
  checkToken(TokenType::Semicolon);
  checkToken(TokenType::Star);
  checkToken(TokenType::Slash);
  checkToken(TokenType::Bang);
  checkToken(TokenType::BangEqual);
  checkToken(TokenType::Equal);
  checkToken(TokenType::EqualEqual);
  checkToken(TokenType::Less);
  checkToken(TokenType::LessEqual);
  checkToken(TokenType::Greater);
  checkToken(TokenType::GreaterEqual);
  checkToken(TokenType::And);
  checkToken(TokenType::Or);
  checkToken(TokenType::EndOfFile);
}

TEST_F(LexerTest, ScansKeywords) {
  auto lexer = createLexer("ལས་ཀ་ གྲངས་ཀ་ ཡིག་རྟགས་ བདེན་ རྫུན་ གལ་ཏེ་ ཡང་ན་ རིམ་པ་ ལོག་ འབྲི་");
  
  EXPECT_EQ(lexer.next().type, TokenType::KwFunction);
  EXPECT_EQ(lexer.next().type, TokenType::KwNumber);
  EXPECT_EQ(lexer.next().type, TokenType::KwString);
  EXPECT_EQ(lexer.next().type, TokenType::KwBoolean);
  EXPECT_EQ(lexer.next().type, TokenType::KwFalse);
  EXPECT_EQ(lexer.next().type, TokenType::KwIf);
  EXPECT_EQ(lexer.next().type, TokenType::KwElse);
  EXPECT_EQ(lexer.next().type, TokenType::KwLoop);
  EXPECT_EQ(lexer.next().type, TokenType::KwReturn);
  EXPECT_EQ(lexer.next().type, TokenType::KwPrint);
  EXPECT_EQ(lexer.next().type, TokenType::EndOfFile);
}

TEST_F(LexerTest, ScansNumbers) {
  auto lexer = createLexer("123 ༠༡༢༣༤༥༦༧༨༩");
  
  auto t1 = lexer.next();
  EXPECT_EQ(t1.type, TokenType::Number);
  EXPECT_EQ(t1.text(lexer.source()), "123");

  auto t2 = lexer.next();
  EXPECT_EQ(t2.type, TokenType::Number);
  EXPECT_EQ(t2.text(lexer.source()), "༠༡༢༣༤༥༦༧༨༩");
}

TEST_F(LexerTest, ScansIdentifiers) {
  auto lexer = createLexer("var_name བོད་ཡིག་");
  
  auto t1 = lexer.next();
  EXPECT_EQ(t1.type, TokenType::Identifier);
  EXPECT_EQ(t1.text(lexer.source()), "var_name");

  auto t2 = lexer.next();
  EXPECT_EQ(t2.type, TokenType::Identifier);
  EXPECT_EQ(t2.text(lexer.source()), "བོད་ཡིག་");
}
