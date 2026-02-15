#pragma once

#include "druk/lexer/lexer.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/node.hpp"
#include "druk/parser/ast/stmt.hpp"
#include <vector>


namespace druk {

class Parser {
public:
  Parser(std::string_view source, ArenaAllocator &arena,
         StringInterner &interner, ErrorReporter &errors);

  std::vector<Stmt *> parse();

private:
  Stmt *parse_declaration();
  Stmt *parse_function();
  Stmt *parse_var_declaration();
  Stmt *parse_statement();
  Stmt *parse_if_statement();
  Stmt *parse_loop_statement();
  Stmt *parse_return_statement();
  Stmt *parse_expression_statement();
  Stmt *parse_block();

  Expr *parse_expression();
  Expr *parse_assignment();
  Expr *parse_logic_or();
  Expr *parse_logic_and();
  Expr *parse_equality();
  Expr *parse_comparison();
  Expr *parse_term();
  Expr *parse_factor();
  Expr *parse_unary();
  Expr *parse_call();
  Expr *parse_primary();

  // Helpers
  bool match(TokenKind kind);
  bool check(TokenKind kind) const;
  Token advance();
  Token consume(TokenKind kind, std::string_view message);
  Token peek() const;
  Token previous() const;
  bool is_at_end() const;
  void synchronize();
  void error(Token token, std::string_view message);

  Lexer lexer_;
  ArenaAllocator &arena_;
  ErrorReporter &errors_;

  std::vector<Token> tokens_; // Buffer? Or streaming?
  // Streaming approach:
  Token current_;
  Token previous_;
  bool panic_mode_ = false;
};

} // namespace druk
