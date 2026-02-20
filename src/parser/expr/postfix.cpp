#include "druk/parser/core/parser.hpp"

namespace druk::parser {

ast::Expr *Parser::parsePostfix() {
  ast::Expr *expr = parsePrimary();

  while (true) {
    if (match(lexer::TokenType::LParen)) {
      expr = parseCall(expr);
    } else if (match(lexer::TokenType::LBracket)) {
      lexer::Token bracket = previous();
      ast::Expr *index = parseExpression();
      consume(lexer::TokenType::RBracket, "Expect ']' after index.");
      auto *idx = arena_.make<ast::IndexExpr>();
      idx->kind = ast::NodeKind::Index;
      idx->array = expr;
      idx->index = index;
      idx->token = bracket;
      expr = idx;
    } else if (match(lexer::TokenType::Dot)) {
      lexer::Token name = consume(lexer::TokenType::Identifier, "Expect property name after '.'.");
      auto *member = arena_.make<ast::MemberAccessExpr>();
      member->kind = ast::NodeKind::MemberAccess;
      member->object = expr;
      member->memberName = name;
      expr = member;
    } else if (match(lexer::TokenType::Bang)) {
      auto *unwrap = arena_.make<ast::UnwrapExpr>();
      unwrap->kind = ast::NodeKind::UnwrapExpr;
      unwrap->operand = expr;
      unwrap->token = previous();
      expr = unwrap;
    } else {
      break;
    }
  }

  return expr;
}

ast::Expr *Parser::parseCall(ast::Expr *callee) {
  std::vector<ast::Expr *> args;
  if (!check(lexer::TokenType::RParen)) {
    do {
      args.push_back(parseExpression());
    } while (match(lexer::TokenType::Comma));
  }

  lexer::Token paren = consume(lexer::TokenType::RParen, "Expect ')' after arguments.");

  auto *call = arena_.make<ast::CallExpr>();
  call->kind = ast::NodeKind::Call;
  call->callee = callee;
  call->token = paren;

  if (args.empty()) {
    call->args = nullptr;
    call->argCount = 0;
  } else {
    call->argCount = static_cast<uint32_t>(args.size());
    call->args = reinterpret_cast<ast::Node **>(arena_.allocateArray<ast::Expr *>(call->argCount));
    for (size_t i = 0; i < args.size(); ++i) {
      call->args[i] = args[i];
    }
  }

  return call;
}

} // namespace druk::parser
