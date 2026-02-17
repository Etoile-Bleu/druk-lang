#include "druk/parser/core/parser.hpp"

namespace druk::parser {

ast::Stmt *Parser::parseDeclaration() {
  if (match(lexer::TokenType::KwFunction)) {
    return parseFunction();
  }

  if (match(lexer::TokenType::KwNumber) || match(lexer::TokenType::KwString) ||
      match(lexer::TokenType::KwBoolean)) {
    return parseVarDeclaration();
  }

  return parseStatement();
}

ast::Stmt *Parser::parseFunction() {
  lexer::Token name = consume(lexer::TokenType::Identifier, "Expect function name.");
  consume(lexer::TokenType::LParen, "Expect '(' after function name.");

  std::vector<lexer::Token> params;
  if (!check(lexer::TokenType::RParen)) {
    do {
      if (match(lexer::TokenType::KwNumber) || match(lexer::TokenType::KwString) ||
          match(lexer::TokenType::KwBoolean)) {
        // Built-in type
      } else {
        consume(lexer::TokenType::Identifier, "Expect parameter type.");
      }

      lexer::Token paramName =
          consume(lexer::TokenType::Identifier, "Expect parameter name.");
      params.push_back(paramName);
    } while (match(lexer::TokenType::Comma));
  }

  consume(lexer::TokenType::RParen, "Expect ')' after parameters.");
  ast::Stmt *body = parseBlock();

  auto *func = arena_.make<ast::FuncDecl>();
  func->kind = ast::NodeKind::Function;
  func->name = name;
  func->body = body;

  if (params.empty()) {
    func->params = nullptr;
    func->paramCount = 0;
  } else {
    func->paramCount = static_cast<uint32_t>(params.size());
    func->params = arena_.allocateArray<lexer::Token>(func->paramCount);
    for (size_t i = 0; i < params.size(); ++i) {
      func->params[i] = params[i];
    }
  }

  return func;
}

ast::Stmt *Parser::parseVarDeclaration() {
  lexer::Token typeToken = previous();
  lexer::Token name = consume(lexer::TokenType::Identifier, "Expect variable name.");

  ast::Expr *initializer = nullptr;
  if (match(lexer::TokenType::Equal)) {
    initializer = parseExpression();
  }

  consume(lexer::TokenType::Semicolon, "Expect ';' after variable declaration.");

  auto *var = arena_.make<ast::VarDecl>();
  var->kind = ast::NodeKind::Variable;
  var->name = name;
  var->initializer = initializer;
  var->typeToken = typeToken;
  return var;
}

} // namespace druk::parser
