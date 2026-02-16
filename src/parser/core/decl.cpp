#include "druk/parser/core/parser.hpp"

namespace druk {

Stmt *Parser::parse_declaration() {
  if (match(TokenKind::KwFunction))
    return parse_function();
  
  // Allow top-level statements for testing/REPL mode
  // In a strict implementation, only functions would be allowed at top level
  return parse_statement();
}

Stmt *Parser::parse_function() {
  // Already consumed KwFunction
  Token name = consume(TokenKind::Identifier, "Expect function name.");
  consume(TokenKind::LParen, "Expect '(' after function name.");

  // Parse parameters
  if (!check(TokenKind::RParen)) {
    do {
      consume(TokenKind::Identifier, "Expect parameter type."); // Type name
      consume(TokenKind::Identifier, "Expect parameter name.");
    } while (match(TokenKind::Comma));
  }

  consume(TokenKind::RParen, "Expect ')' after parameters.");

  consume(TokenKind::LBrace, "Expect '{' before function body.");
  Stmt *body = parse_block();

  auto *func = arena_.make<FuncDecl>();
  func->kind = NodeKind::Function;
  func->name = name;
  func->body = body;
  // param storing omitted for brevity/allocator simplicity in this pass
  return func;
}

Stmt *Parser::parse_var_declaration() {
  // Token type = previous(); // Consumed by caller (parse_statement)
  // We might want to store type in VarDecl later. For now ignore.
  Token name = consume(TokenKind::Identifier, "Expect variable name.");

  Expr *initializer = nullptr;
  if (match(TokenKind::Equal)) {
    initializer = parse_expression();
  }

  consume(TokenKind::Semicolon, "Expect ';' after variable declaration.");

  auto *var = arena_.make<VarDecl>();
  var->kind = NodeKind::Variable;
  var->name = name;
  var->initializer = initializer;
  return var;
}

} // namespace druk
