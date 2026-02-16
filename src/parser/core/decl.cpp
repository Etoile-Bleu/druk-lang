#include "druk/parser/core/parser.hpp"

namespace druk {

Stmt *Parser::parse_declaration() {
  if (match(TokenKind::KwFunction)) {
    return parse_function();
  }

  // Check for variable declaration start (Types)
  if (match(TokenKind::KwNumber) || match(TokenKind::KwString) ||
      match(TokenKind::KwBoolean)) {
    return parse_var_declaration();
  }

  // Otherwise assume statement
  // But wait, parse_statement also needs to be implemented.
  // If we don't have parse_statement, we panic.
  // For now, let's try parse_statement.
  return parse_statement();
}

Stmt *Parser::parse_function() {
  // Already consumed KwFunction
  Token name = consume(TokenKind::Identifier, "Expect function name.");
  consume(TokenKind::LParen, "Expect '(' after function name.");

  std::vector<Token> params;
  // Parse parameters
  if (!check(TokenKind::RParen)) {
    do {
      // Consume Type (Keyword or Identifier)
      if (match(TokenKind::KwNumber) || match(TokenKind::KwString) ||
          match(TokenKind::KwBoolean)) {
        // Built-in type
      } else {
        consume(TokenKind::Identifier, "Expect parameter type.");
      }

      Token param_name =
          consume(TokenKind::Identifier, "Expect parameter name.");
      params.push_back(param_name);

      // Store param (we used to only have FuncDecl structure with Token name...
      // check if we need to store params in AST now?)
      // FuncDecl in stmt.hpp has `Token *params`.
      // We should probably store them.
      // But for now just consume appropriately to avoid infinite loop.

      // Update: we need to store them in a temporary vector to put into
      // FuncDecl later But first let's fix the loop hang.
    } while (match(TokenKind::Comma));
  }

  consume(TokenKind::RParen, "Expect ')' after parameters.");

  Stmt *body = parse_block();

  auto *func = arena_.make<FuncDecl>();
  func->kind = NodeKind::Function;
  func->name = name;
  func->body = body;

  if (params.empty()) {
    func->params = nullptr;
    func->param_count = 0;
  } else {
    func->param_count = static_cast<uint32_t>(params.size());
    func->params = arena_.allocate_array<Token>(func->param_count);
    for (size_t i = 0; i < params.size(); ++i) {
      func->params[i] = params[i];
    }
  }

  return func;
}

Stmt *Parser::parse_var_declaration() {
  Token type_token = previous(); // Consumed by caller (parse_statement/decl)
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
  var->type_token = type_token;
  return var;
}

} // namespace druk
