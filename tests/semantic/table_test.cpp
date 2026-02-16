#include "druk/semantic/table.hpp"
#include <gtest/gtest.h>


TEST(SymbolTableTest, BasicScope) {
  druk::SymbolTable table;
  druk::Token token;
  token.kind = druk::TokenKind::Identifier;

  // Global scope
  EXPECT_TRUE(table.define("x", {token, druk::Type::Int()}));
  EXPECT_FALSE(
      table.define("x", {token, druk::Type::Int()})); // Redefinition fail

  auto *sym = table.resolve("x");
  ASSERT_NE(sym, nullptr);
  EXPECT_EQ(sym->type, druk::Type::Int());

  // Nested scope
  table.enter_scope();
  EXPECT_TRUE(table.define(
      "x", {token,
            druk::Type::String()})); // Shadowing allowed? Assuming yes for now
  sym = table.resolve("x");
  ASSERT_NE(sym, nullptr);
  EXPECT_EQ(sym->type, druk::Type::String());

  table.exit_scope();
  sym = table.resolve("x");
  ASSERT_NE(sym, nullptr);
  EXPECT_EQ(sym->type, druk::Type::Int());
}
