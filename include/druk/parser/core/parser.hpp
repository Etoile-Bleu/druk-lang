#pragma once

#include "druk/lexer/lexer.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/node.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/util/arena_allocator.hpp"
#include "druk/util/error_handler.hpp"
#include <vector>

namespace druk::parser {

/**
 * @brief Parser converts a stream of tokens into an Abstract Syntax Tree (AST).
 */
class Parser {
public:
  Parser(std::string_view source, util::ArenaAllocator &arena,
         lexer::StringInterner &interner, util::ErrorHandler &errors);

  /**
   * @brief Parses the entire source into a list of statements.
   * @return A vector of statement pointers managed by the arena.
   */
  std::vector<ast::Stmt *> parse();

private:
  // Declarations
  ast::Stmt *parseDeclaration();
  ast::Stmt *parseFunction();
  ast::Stmt *parseVarDeclaration();

  // Statements
  ast::Stmt *parseStatement();
  ast::Stmt *parseIfStatement();
  ast::Stmt *parseLoopStatement();
  ast::Stmt *parseReturnStatement();
  ast::Stmt *parsePrintStatement();
  ast::Stmt *parseExpressionStatement();
  ast::Stmt *parseBlock();

  // Expressions
  ast::Expr *parseExpression();
  ast::Expr *parseAssignment();
  ast::Expr *parseLogicalOr();
  ast::Expr *parseLogicalAnd();
  ast::Expr *parseEquality();
  ast::Expr *parseComparison();
  ast::Expr *parseTerm();
  ast::Expr *parseFactor();
  ast::Expr *parseUnary();
  ast::Expr *parseCall(ast::Expr *callee);
  ast::Expr *parsePostfix();
  ast::Expr *parsePrimary();

  // Collections
  ast::Expr *parseArrayLiteral();
  ast::Expr *parseStructLiteral();

  // Helpers
  bool match(lexer::TokenType kind);
  bool check(lexer::TokenType kind) const;
  lexer::Token advance();
  lexer::Token consume(lexer::TokenType kind, std::string_view message);
  lexer::Token peek() const;
  lexer::Token previous() const;
  bool isAtEnd() const;
  void synchronize();
  void error(lexer::Token token, std::string_view message);

  lexer::Lexer lexer_;
  util::ArenaAllocator &arena_;
  lexer::StringInterner &interner_;
  util::ErrorHandler &errors_;

  lexer::Token current_;
  lexer::Token previous_;
  bool panicMode_ = false;
};

} // namespace druk::parser
