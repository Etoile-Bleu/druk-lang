#include <gtest/gtest.h>

#include <iostream>

#include "druk/parser/ast/lambda.hpp"
#include "druk/parser/ast/match.hpp"
#include "helpers/test_helpers.h"


namespace druk::parser::test
{

using namespace druk::test;

TEST(ParserFeatureTest, TokenizeMatch)
{
    LexHelper helper;
    auto      tokens = helper.tokenize("འགྲིག་པ་");
    ASSERT_EQ(tokens.size(), 1);
    ASSERT_EQ(tokens[0].type, druk::lexer::TokenType::KwMatch);
}

TEST(ParserFeatureTest, ParseLambdaBasic)
{
    ParseHelper      helper;
    std::string_view source = "(a, b) -> a + b;";
    auto             stmts  = helper.parse(source);
    ASSERT_FALSE(helper.hasErrors());
    ASSERT_EQ(stmts.size(), 1);
}

TEST(ParserFeatureTest, ParseMatchBasic)
{
    std::string_view source = "འགྲིག་པ་ x { 1 -> བཀོད་ 1; _ -> བཀོད་ 0; }";
    ParseHelper      helper;
    auto             stmts = helper.parse(source);
    ASSERT_FALSE(helper.hasErrors());
    ASSERT_EQ(stmts.size(), 1);
}

TEST(SemanticFeatureTest, AnalyzeLambda)
{
    SemanticHelper helper;
    ASSERT_TRUE(helper.analyze("(a) -> a + 1;"));
}

TEST(SemanticFeatureTest, AnalyzeMatch)
{
    SemanticHelper helper;
    ASSERT_TRUE(helper.analyze("གྲངས་ x = 1; འགྲིག་པ་ x { 1 -> བཀོད་ 1; _ -> བཀོད་ 0; }"));
}

}  // namespace druk::parser::test
