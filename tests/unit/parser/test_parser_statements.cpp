// test_parser_statements.cpp — VarDecl, Print, Return, ExpressionStmt
#include <gtest/gtest.h>

#include "druk/parser/ast/type.hpp"
#include "helpers/test_helpers.h"

using namespace druk::test;
using namespace druk::parser::ast;

class ParserStatementsTest : public ::testing::Test
{
   protected:
    ParseHelper ph;
};

// ─── Variable declarations ────────────────────────────────────────────────────

TEST_F(ParserStatementsTest, NumberDeclaration)
{
    auto stmts = ph.parse("གྲངས་ x = 10;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* decl = dynamic_cast<VarDecl*>(stmts[0]);
    ASSERT_TRUE(decl);
    auto* type = dynamic_cast<BuiltinType*>(decl->type);
    ASSERT_TRUE(type);
    EXPECT_EQ(type->token.type, TT::KwNumber);
}

TEST_F(ParserStatementsTest, StringDeclaration)
{
    auto stmts = ph.parse("ཡིག་འབྲུ་ name = \"druk\";");
    ASSERT_EQ(stmts.size(), 1u);
    auto* decl = dynamic_cast<VarDecl*>(stmts[0]);
    ASSERT_TRUE(decl);
    auto* type = dynamic_cast<BuiltinType*>(decl->type);
    ASSERT_TRUE(type);
    EXPECT_EQ(type->token.type, TT::KwString);
}

TEST_F(ParserStatementsTest, BooleanDeclaration)
{
    auto stmts = ph.parse("བདེན་རྫུན་ flag = བདེན་པ་;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* decl = dynamic_cast<VarDecl*>(stmts[0]);
    ASSERT_TRUE(decl);
    auto* type = dynamic_cast<BuiltinType*>(decl->type);
    ASSERT_TRUE(type);
    EXPECT_EQ(type->token.type, TT::KwBoolean);
}

TEST_F(ParserStatementsTest, DeclarationWithInitializerHasCorrectValue)
{
    auto  stmts = ph.parse("གྲངས་ x = 42;");
    auto* decl  = dynamic_cast<VarDecl*>(stmts[0]);
    ASSERT_TRUE(decl);
    auto* lit = dynamic_cast<LiteralExpr*>(decl->initializer);
    ASSERT_TRUE(lit);
    EXPECT_EQ(lit->literalValue.asInt(), 42);
}

TEST_F(ParserStatementsTest, MultipleDeclarations)
{
    auto stmts = ph.parse("གྲངས་ a = 1; གྲངས་ b = 2;");
    ASSERT_EQ(stmts.size(), 2u);
    EXPECT_TRUE(dynamic_cast<VarDecl*>(stmts[0]));
    EXPECT_TRUE(dynamic_cast<VarDecl*>(stmts[1]));
}

// ─── Print statement ──────────────────────────────────────────────────────────

TEST_F(ParserStatementsTest, PrintStatement)
{
    auto stmts = ph.parse("བཀོད་ 42;");
    ASSERT_EQ(stmts.size(), 1u);
    auto* print = dynamic_cast<PrintStmt*>(stmts[0]);
    ASSERT_TRUE(print);
}

TEST_F(ParserStatementsTest, PrintStringLiteral)
{
    auto  stmts = ph.parse("བཀོད་ \"hello\";");
    auto* print = dynamic_cast<PrintStmt*>(stmts[0]);
    ASSERT_TRUE(print);
    auto* lit = dynamic_cast<LiteralExpr*>(print->expression);
    ASSERT_TRUE(lit);
    EXPECT_TRUE(lit->literalValue.isString());
}

// ─── Return statement ─────────────────────────────────────────────────────────

TEST_F(ParserStatementsTest, ReturnWithValue)
{
    auto  stmts = ph.parse("ལས་འགན་ f() { སླར་ལོག་ 1; }");
    auto* func  = dynamic_cast<FuncDecl*>(stmts[0]);
    ASSERT_TRUE(func);
    auto* body = dynamic_cast<BlockStmt*>(func->body);
    ASSERT_TRUE(body);
    ASSERT_EQ(body->count, 1u);
    auto* ret = dynamic_cast<ReturnStmt*>(body->statements[0]);
    ASSERT_TRUE(ret);
    EXPECT_TRUE(ret->value != nullptr);
}

// ─── Expression statement ─────────────────────────────────────────────────────

TEST_F(ParserStatementsTest, ExpressionStatement)
{
    auto stmts = ph.parse("1 + 2;");
    ASSERT_EQ(stmts.size(), 1u);
    EXPECT_TRUE(dynamic_cast<ExpressionStmt*>(stmts[0]));
}

TEST_F(ParserStatementsTest, NoErrorsOnValidStatements)
{
    ph.parse("གྲངས་ x = 5; བཀོད་ x;");
    EXPECT_TRUE(ph.noErrors());
}
