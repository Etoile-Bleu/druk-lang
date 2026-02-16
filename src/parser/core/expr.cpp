#include "druk/parser/core/parser.hpp"
#include <vector>

namespace druk {

Expr *Parser::parse_expression() { return parse_assignment(); }

Expr *Parser::parse_assignment() {
  Expr *expr = parse_logic_or();

  if (match(TokenKind::Equal)) {
    Token equals = previous();
    Expr *value = parse_assignment();

    if (expr->kind == NodeKind::VariableExpr ||
        expr->kind == NodeKind::Index ||
        expr->kind == NodeKind::MemberAccess) {
      auto *assign = arena_.make<AssignmentExpr>();
      assign->kind = NodeKind::Assignment;
      assign->target = expr;
      assign->value = value;
      assign->token = equals;
      return assign;
    }

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

Expr *Parser::parse_logic_or() {
  Expr *expr = parse_logic_and();
  while (match(TokenKind::Or)) {
    Token op = previous();
    Expr *right = parse_logic_and();
    auto *node = arena_.make<LogicalExpr>();
    node->kind = NodeKind::Logical;
    node->left = expr;
    node->right = right;
    node->token = op;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_logic_and() {
  Expr *expr = parse_equality();
  while (match(TokenKind::And)) {
    Token op = previous();
    Expr *right = parse_equality();
    auto *node = arena_.make<LogicalExpr>();
    node->kind = NodeKind::Logical;
    node->left = expr;
    node->right = right;
    node->token = op;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_equality() {
  Expr *expr = parse_comparison();
  while (match(TokenKind::BangEqual) || match(TokenKind::EqualEqual)) {
    Token op = previous();
    Expr *right = parse_comparison();
    auto *node = arena_.make<BinaryExpr>();
    node->kind = NodeKind::Binary;
    node->left = expr;
    node->right = right;
    node->token = op;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_comparison() {
  Expr *expr = parse_term();
  while (match(TokenKind::Greater) || match(TokenKind::GreaterEqual) ||
         match(TokenKind::Less) || match(TokenKind::LessEqual)) {
    Token op = previous();
    Expr *right = parse_term();
    auto *node = arena_.make<BinaryExpr>();
    node->kind = NodeKind::Binary;
    node->left = expr;
    node->right = right;
    node->token = op;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_term() {
  Expr *expr = parse_factor();
  while (match(TokenKind::Plus) || match(TokenKind::Minus)) {
    Token op = previous();
    Expr *right = parse_factor();
    auto *node = arena_.make<BinaryExpr>();
    node->kind = NodeKind::Binary;
    node->left = expr;
    node->right = right;
    node->token = op;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_factor() {
  Expr *expr = parse_unary();
  while (match(TokenKind::Star) || match(TokenKind::Slash)) {
    Token op = previous();
    Expr *right = parse_unary();
    auto *node = arena_.make<BinaryExpr>();
    node->kind = NodeKind::Binary;
    node->left = expr;
    node->right = right;
    node->token = op;
    expr = node;
  }
  return expr;
}

Expr *Parser::parse_unary() {
  if (match(TokenKind::Bang) || match(TokenKind::Minus)) {
    Token op = previous();
    Expr *right = parse_unary();
    auto *node = arena_.make<UnaryExpr>();
    node->kind = NodeKind::Unary;
    node->right = right;
    node->token = op;
    return node;
  }
  return parse_postfix();
}

Expr *Parser::parse_postfix() {
  Expr *expr = parse_primary();

  while (true) {
    if (match(TokenKind::LParen)) {
      // Function call
      std::vector<Expr *> args;
      if (!check(TokenKind::RParen)) {
        do {
          args.push_back(parse_expression());
        } while (match(TokenKind::Comma));
      }

      Token paren = consume(TokenKind::RParen, "Expect ')' after arguments.");
      auto *call = arena_.make<CallExpr>();
      call->kind = NodeKind::Call;
      call->callee = expr;
      call->token = paren;
      call->arg_count = static_cast<uint32_t>(args.size());
      if (call->arg_count > 0) {
        call->args = arena_.allocate_array<Node *>(args.size());
        for (size_t i = 0; i < args.size(); ++i) {
          call->args[i] = args[i];
        }
      } else {
        call->args = nullptr;
      }
      expr = call;
      
    } else if (match(TokenKind::LBracket)) {
      // Array indexing: expr[index]
      Expr *index = parse_expression();
      consume(TokenKind::RBracket, "Expect ']' after index.");

      auto *idx = arena_.make<IndexExpr>();
      idx->kind = NodeKind::Index;
      idx->array = expr;
      idx->index = index;
      expr = idx;

    } else if (match(TokenKind::Dot)) {
      // Member access: expr.member
      Token member = consume(TokenKind::Identifier, "Expect property name after '.'.");

      auto *mem = arena_.make<MemberAccessExpr>();
      mem->kind = NodeKind::MemberAccess;
      mem->object = expr;
      mem->member_name = member;
      expr = mem;

    } else {
      break;
    }
  }

  return expr;
}

Expr *Parser::parse_call() {
  // Deprecated: now using parse_postfix
  return parse_postfix();
}

Expr *Parser::parse_primary() {
  if (match(TokenKind::Number)) {
    std::string text(previous().text(lexer_.source()));
    std::string ascii_text;
    for (size_t i = 0; i < text.size(); ++i) {
      unsigned char c = static_cast<unsigned char>(text[i]);
      if (c == 0xE0 && i + 2 < text.size()) {
        unsigned char n1 = static_cast<unsigned char>(text[i + 1]);
        unsigned char n2 = static_cast<unsigned char>(text[i + 2]);
        if (n1 == 0xBC && (n2 >= 0xA0 && n2 <= 0xA9)) {
          ascii_text += (char)('0' + (n2 - 0xA0));
          i += 2;
          continue;
        }
      }
      ascii_text += text[i];
    }
    try {
      int64_t value = std::stoll(ascii_text);
      auto *lit = arena_.make<LiteralExpr>();
      lit->kind = NodeKind::Literal;
      lit->token = previous();
      lit->literal_value = Value(value);
      return lit;
    } catch (...) {
      error(previous(), "Invalid number literal.");
      return nullptr;
    }
  }

  if (match(TokenKind::String)) {
    auto *lit = arena_.make<LiteralExpr>();
    lit->kind = NodeKind::Literal;
    lit->token = previous();
    std::string_view text = previous().text(lexer_.source());
    if (text.length() >= 2) {
      lit->literal_value = Value(interner_.intern(text.substr(1, text.length() - 2)));
    } else {
      lit->literal_value = Value(std::string_view(""));
    }
    return lit;
  }

  if (match(TokenKind::KwTrue) || match(TokenKind::KwBoolean)) {
    auto *lit = arena_.make<LiteralExpr>();
    lit->kind = NodeKind::Literal;
    lit->token = previous();
    lit->literal_value = Value(true);
    return lit;
  }

  if (match(TokenKind::KwFalse)) {
    auto *lit = arena_.make<LiteralExpr>();
    lit->kind = NodeKind::Literal;
    lit->token = previous();
    lit->literal_value = Value(false);
    return lit;
  }

  if (match(TokenKind::LParen)) {
    Expr *expr = parse_expression();
    consume(TokenKind::RParen, "Expect ')' after expression.");
    auto *group = arena_.make<GroupingExpr>();
    group->kind = NodeKind::Grouping;
    group->expression = expr;
    group->token = previous(); // Store closing paren
    return group;
  }

  if (match(TokenKind::Identifier)) {
    auto *var = arena_.make<VariableExpr>();
    var->kind = NodeKind::VariableExpr;
    var->name = previous();
    var->token = previous();
    return var;
  }

  // Array literal: [1, 2, 3]
  if (match(TokenKind::LBracket)) {
    return parse_array_literal();
  }

  // Struct literal: { field: value }
  // In expression context, { is struct literal
  if (check(TokenKind::LBrace)) {
    advance(); // consume '{'
    return parse_struct_literal();
  }

  error(peek(), "Expect expression.");
  return nullptr;
}

// Parse array literal: [1, 2, 3]
Expr *Parser::parse_array_literal() {
  // Already consumed '['
  std::vector<Expr *> elements;

  if (!check(TokenKind::RBracket)) {
    do {
      elements.push_back(parse_expression());
    } while (match(TokenKind::Comma));
  }

  consume(TokenKind::RBracket, "Expect ']' after array elements.");

  auto *arr = arena_.make<ArrayLiteralExpr>();
  arr->kind = NodeKind::ArrayLiteral;
  arr->count = static_cast<uint32_t>(elements.size());
  arr->elements = arena_.make_array<Expr *>(elements.size());
  for (size_t i = 0; i < elements.size(); i++) {
    arr->elements[i] = elements[i];
  }

  return arr;
}

// Parse struct literal: { name: "John", age: 25 }
Expr *Parser::parse_struct_literal() {
  // Already consumed '{'
  std::vector<Token> field_names;
  std::vector<Expr *> field_values;

  if (!check(TokenKind::RBrace)) {
    do {
      Token field_name = consume(TokenKind::Identifier, "Expect field name.");
      field_names.push_back(field_name);
      consume(TokenKind::Colon, "Expect ':' after field name.");
      Expr *value = parse_expression();
      field_values.push_back(value);
    } while (match(TokenKind::Comma));
  }

  consume(TokenKind::RBrace, "Expect '}' after struct fields.");

  auto *st = arena_.make<StructLiteralExpr>();
  st->kind = NodeKind::StructLiteral;
  st->field_count = static_cast<uint32_t>(field_names.size());
  st->field_names = arena_.make_array<Token>(field_names.size());
  st->field_values = arena_.make_array<Expr *>(field_values.size());
  
  for (size_t i = 0; i < field_names.size(); i++) {
    st->field_names[i] = field_names[i];
    st->field_values[i] = field_values[i];
  }

  return st;
}

} // namespace druk
