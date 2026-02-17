#include "druk/parser/core/parser.hpp"

namespace druk::parser {

ast::Expr *Parser::parseArrayLiteral() {
  consume(lexer::TokenType::LBracket, "Expect '[' and array literal.");
  std::vector<ast::Expr *> elements;
  if (!check(lexer::TokenType::RBracket)) {
    do {
      elements.push_back(parseExpression());
    } while (match(lexer::TokenType::Comma));
  }
  lexer::Token bracket = consume(lexer::TokenType::RBracket, "Expect ']' after array literal.");

  auto *arr = arena_.make<ast::ArrayLiteralExpr>();
  arr->kind = ast::NodeKind::ArrayLiteral;
  arr->token = bracket;
  arr->count = static_cast<uint32_t>(elements.size());
  if (elements.empty()) {
    arr->elements = nullptr;
  } else {
    arr->elements = arena_.allocateArray<ast::Expr *>(arr->count);
    for (size_t i = 0; i < elements.size(); ++i) {
      arr->elements[i] = elements[i];
    }
  }
  return arr;
}

ast::Expr *Parser::parseStructLiteral() {
  consume(lexer::TokenType::LBrace, "Expect '{' and struct literal.");
  std::vector<lexer::Token> names;
  std::vector<ast::Expr *> values;
  if (!check(lexer::TokenType::RBrace)) {
    do {
      lexer::Token name = consume(lexer::TokenType::Identifier, "Expect field name.");
      consume(lexer::TokenType::Colon, "Expect ':' after field name.");
      ast::Expr *value = parseExpression();
      names.push_back(name);
      values.push_back(value);
    } while (match(lexer::TokenType::Comma));
  }
  lexer::Token brace = consume(lexer::TokenType::RBrace, "Expect '}' after struct literal.");

  auto *struc = arena_.make<ast::StructLiteralExpr>();
  struc->kind = ast::NodeKind::StructLiteral;
  struc->token = brace;
  struc->fieldCount = static_cast<uint32_t>(names.size());
  if (names.empty()) {
    struc->fieldNames = nullptr;
    struc->fieldValues = nullptr;
  } else {
    struc->fieldNames = arena_.allocateArray<lexer::Token>(struc->fieldCount);
    struc->fieldValues = arena_.allocateArray<ast::Expr *>(struc->fieldCount);
    for (size_t i = 0; i < names.size(); ++i) {
      struc->fieldNames[i] = names[i];
      struc->fieldValues[i] = values[i];
    }
  }
  return struc;
}

} // namespace druk::parser
