// test_parser_control_flow.cpp — If/else, while, for, legacy loop
#include <gtest/gtest.h>

#include "helpers/test_helpers.h"


using namespace druk::test;
using namespace druk::parser::ast;

class ParserControlFlowTest : public ::testing::Test
{
   protected:
    ParseHelper ph;
};

// ─── If statement ─────────────────────────────────────────────────────────────

TEST_F(ParserControlFlowTest, IfThenOnly)
{
    auto stmts = ph.parse("གལ་སྲིད་ (༡ < ༢) { བཀོད་ ༡; }");
    ASSERT_EQ(stmts.size(), 1u);
    auto* ifs = dynamic_cast<IfStmt*>(stmts[0]);
    ASSERT_TRUE(ifs);
    EXPECT_TRUE(ifs->condition != nullptr);
    EXPECT_TRUE(ifs->thenBranch != nullptr);
    EXPECT_TRUE(ifs->elseBranch == nullptr);
}

TEST_F(ParserControlFlowTest, IfElse)
{
    auto  stmts = ph.parse("གལ་སྲིད་ (བདེན་པ་) { བཀོད་ ༡; } མེད་ན་ { བཀོད་ ༠; }");
    auto* ifs   = dynamic_cast<IfStmt*>(stmts[0]);
    ASSERT_TRUE(ifs);
    EXPECT_TRUE(ifs->elseBranch != nullptr);
}

TEST_F(ParserControlFlowTest, IfConditionIsBinaryExpr)
{
    auto  stmts = ph.parse("གལ་སྲིད་ (a == b) { བཀོད་ ༡; }");
    auto* ifs   = dynamic_cast<IfStmt*>(stmts[0]);
    ASSERT_TRUE(ifs);
    auto* cond = dynamic_cast<BinaryExpr*>(ifs->condition);
    ASSERT_TRUE(cond);
    EXPECT_EQ(cond->token.type, TT::EqualEqual);
}

// ─── While loop ───────────────────────────────────────────────────────────────

TEST_F(ParserControlFlowTest, WhileLoop)
{
    auto stmts = ph.parse("ཡང་བསྐྱར་ (x < ༥) { བཀོད་ x; }");
    ASSERT_EQ(stmts.size(), 1u);
    auto* ws = dynamic_cast<WhileStmt*>(stmts[0]);
    ASSERT_TRUE(ws);
    EXPECT_TRUE(ws->condition != nullptr);
    EXPECT_TRUE(ws->body != nullptr);
}

TEST_F(ParserControlFlowTest, WhileBodyIsBlock)
{
    auto  stmts = ph.parse("ཡང་བསྐྱར་ (x < ༣) { བཀོད་ x; }");
    auto* ws    = dynamic_cast<WhileStmt*>(stmts[0]);
    ASSERT_TRUE(ws);
    EXPECT_TRUE(dynamic_cast<BlockStmt*>(ws->body));
}

// ─── For loop ─────────────────────────────────────────────────────────────────

TEST_F(ParserControlFlowTest, ForLoopAllClauses)
{
    auto stmts = ph.parse("རེ་རེར་ (གྲངས་ i = ༠; i < ༥; i = i + ༡) { བཀོད་ i; }");
    ASSERT_EQ(stmts.size(), 1u);
    auto* fs = dynamic_cast<ForStmt*>(stmts[0]);
    ASSERT_TRUE(fs);
    EXPECT_TRUE(fs->init != nullptr);
    EXPECT_TRUE(fs->condition != nullptr);
    EXPECT_TRUE(fs->step != nullptr);
    EXPECT_TRUE(fs->body != nullptr);
}

TEST_F(ParserControlFlowTest, ForLoopInitIsVarDecl)
{
    auto  stmts = ph.parse("རེ་རེར་ (གྲངས་ i = ༠; i < ༣; i = i + ༡) { }");
    auto* fs    = dynamic_cast<ForStmt*>(stmts[0]);
    ASSERT_TRUE(fs);
    EXPECT_TRUE(dynamic_cast<VarDecl*>(fs->init));
}

// ─── Legacy loop (རིམ་པ་ → KwLoop) ───────────────────────────────────────────

TEST_F(ParserControlFlowTest, LegacyLoopKeyword)
{
    auto stmts = ph.parse("རིམ་པ་ (x < ༤) { བཀོད་ x; }");
    ASSERT_EQ(stmts.size(), 1u);
    auto* ls = dynamic_cast<LoopStmt*>(stmts[0]);
    ASSERT_TRUE(ls);
    EXPECT_TRUE(ls->condition != nullptr);
}

// ─── No errors ────────────────────────────────────────────────────────────────

TEST_F(ParserControlFlowTest, NoErrorsOnValidIf)
{
    ph.parse("གལ་སྲིད་ (བདེན་པ་) { བཀོད་ ༡; }");
    EXPECT_TRUE(ph.noErrors());
}
TEST_F(ParserControlFlowTest, NoErrorsOnValidFor)
{
    ph.parse("རེ་རེར་ (གྲངས་ i = ༠; i < ༥; i = i + ༡) { བཀོད་ i; }");
    EXPECT_TRUE(ph.noErrors());
}
