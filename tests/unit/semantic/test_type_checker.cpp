// test_type_checker.cpp — Semantic type analysis: valid programs pass, bad programs fail
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;

class TypeCheckerTest : public ::testing::Test
{
   protected:
    SemanticHelper sh;
};

// ─── Valid programs ───────────────────────────────────────────────────────────

TEST_F(TypeCheckerTest, SimpleNumberDeclaration)
{
    EXPECT_TRUE(sh.analyze("གྲངས་ x = ༥;"));
}

TEST_F(TypeCheckerTest, StringDeclaration)
{
    EXPECT_TRUE(sh.analyze("ཡིག་འབྲུ་ s = \"hello\";"));
}

TEST_F(TypeCheckerTest, BoolDeclaration)
{
    EXPECT_TRUE(sh.analyze("བདེན་རྫུན་ flag = བདེན་པ་;"));
}

TEST_F(TypeCheckerTest, PrintNumber)
{
    EXPECT_TRUE(sh.analyze("གྲངས་ n = ༣; བཀོད་ n;"));
}

TEST_F(TypeCheckerTest, IfWithValidCondition)
{
    EXPECT_TRUE(
        sh.analyze("གྲངས་ x = ༡;"
                   "གལ་སྲིད་ (x < ༥) { བཀོད་ x; }"));
}

TEST_F(TypeCheckerTest, WhileLoopValid)
{
    EXPECT_TRUE(
        sh.analyze("གྲངས་ i = ༠;"
                   "ཡང་བསྐྱར་ (i < ༣) { i = i + ༡; }"));
}

TEST_F(TypeCheckerTest, ForLoopValid)
{
    EXPECT_TRUE(sh.analyze("རེ་རེར་ (གྲངས་ i = ༠; i < ༣; i = i + ༡) { བཀོད་ i; }"));
}

TEST_F(TypeCheckerTest, SimpleFunctionValid)
{
    EXPECT_TRUE(sh.analyze("ལས་འགན་ square(གྲངས་ n) { སླར་ལོག་ n * n; }"));
}
