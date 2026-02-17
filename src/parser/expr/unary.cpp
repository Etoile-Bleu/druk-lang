#include "druk/parser/core/parser.hpp"

namespace druk::parser {

ast::Expr *Parser::parseUnary() {
  if (match(lexer::TokenType::Bang) || match(lexer::TokenType::Minus)) {
    lexer::Token op = previous();
    ast::Expr *right = parseUnary();
    auto *unary = arena_.make<ast::UnaryExpr>();
    unary->kind = ast::NodeKind::Unary;
    unary->right = right;
    unary->token = op;
    return unary;
  }

  return parsePostfix();
}

} // namespace druk::parser
