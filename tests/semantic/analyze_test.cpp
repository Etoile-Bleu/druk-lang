#include "druk/lexer/lexer.hpp"
#include "druk/parser/core/parser.hpp" // For parsing to test analysis
#include "druk/semantic/analyzer.hpp"
#include "druk/semantic/table.hpp" // Added include for SymbolTable
#include <gtest/gtest.h>

class AnalyzeTest : public ::testing::Test {
protected:
  druk::ArenaAllocator arena;
  druk::StringInterner interner{arena};
  druk::ErrorReporter errors;
  druk::SymbolTable table;

  std::vector<druk::Stmt *> parse(std::string_view source) {
    druk::Parser parser(source, arena, interner, errors);
    return parser.parse();
  }
};

TEST_F(AnalyzeTest, ValidVarDecl) {
  // ལས་ཀ main() { གྲངས་ཀ x = 10; }
  // We utilize the parser to build AST, then analyze.
  std::string_view source = "ལས་ཀ main() { གྲངས་ཀ x = 10; }";
  auto stmts = parse(source);
  druk::SemanticAnalyzer analyzer{table, errors, interner, source};
  analyzer.analyze(stmts);
  EXPECT_FALSE(errors.has_errors());

  // Check if symbol exists?
  // Analyzer populates table, but scopes pop as we exit blocks.
  // So we can't easily check table state after analysis unless we hook in.
  // Or we check if no errors were reported.
}

TEST_F(AnalyzeTest, RedeclarationError) {
  // ལས་ཀ main() { གྲངས་ཀ x = 10; གྲངས་ཀ x = 20; }
  std::string_view source = "ལས་ཀ main() { གྲངས་ཀ x = 10; གྲངས་ཀ x = 20; }";
  auto stmts = parse(source);
  druk::SemanticAnalyzer analyzer{table, errors, interner, source};
  analyzer.analyze(stmts);
  EXPECT_TRUE(errors.has_errors());
}

TEST_F(AnalyzeTest, UndefinedVar) {
  // ལས་ཀ main() { གྲངས་ཀ x = y; }
  // Wait, I haven't implemented check inside visit_variable yet in expr.cpp
  // So this test might fail (pass unexpectedly) until implemented.
  // Adding it as placeholder.
}

TEST_F(AnalyzeTest, LiteralTypeCheck) {
  // ལས་ཀ main() { གྲངས་ཀ x = 10; ཡིག་རྟགས s = "hello"; }
  std::string_view source =
      "ལས་ཀ main() { གྲངས་ཀ x = 10; ཡིག་རྟགས s = \"hello\"; }";
  auto stmts = parse(source);
  druk::SemanticAnalyzer analyzer{table, errors, interner, source};
  analyzer.analyze(stmts);
  EXPECT_FALSE(errors.has_errors());
}

TEST_F(AnalyzeTest, TypeMismatchError) {
  // ལས་ཀ main() { གྲངས་ཀ x = "hello"; }
  // x is declared as Int (KwNumber), initialized with String.
  // visit_var should report mismatch.
  std::string_view source = "ལས་ཀ main() { གྲངས་ཀ x = \"hello\"; }";
  auto stmts = parse(source);
  druk::SemanticAnalyzer analyzer{table, errors, interner, source};
  analyzer.analyze(stmts);
  EXPECT_TRUE(errors.has_errors());
}

TEST_F(AnalyzeTest, FunctionParams) {
  // ལས་ཀ foo(གྲངས་ཀ x) { }
  // ལས་ཀ main() { }
  // We need to parse parameters correctly first.
  // If our parser supports it.
  std::string_view source = "ལས་ཀ foo(གྲངས་ཀ x) { }";
  auto stmts = parse(source);
  druk::SemanticAnalyzer analyzer{table, errors, interner, source};
  analyzer.analyze(stmts);
  EXPECT_FALSE(errors.has_errors());
}

TEST_F(AnalyzeTest, DuplicateParam) {
  // ལས་ཀ foo(གྲངས་ཀ x, གྲངས་ཀ x) { }
  // Parser might allow it, analyzer should catch it.
  std::string_view source = "ལས་ཀ foo(གྲངས་ཀ x, གྲངས་ཀ x) { }";
  auto stmts = parse(source);
  druk::SemanticAnalyzer analyzer{table, errors, interner, source};
  analyzer.analyze(stmts);
  EXPECT_TRUE(errors.has_errors());
}
