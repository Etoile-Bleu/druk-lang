#include "druk/parser/core/parser.hpp"

namespace druk {

Stmt *Parser::parse_statement() {
  if (match(TokenKind::KwIf))
    return parse_if_statement();
  if (match(TokenKind::KwLoop))
    return parse_loop_statement();
  if (match(TokenKind::KwReturn))
    return parse_return_statement();
  if (match(TokenKind::LBrace))
    return parse_block();

  // Var decl check: type identifier
  // We need lookahead to distinguish expr stmt vs var decl?
  // Or check if current token is a type keyword?
  // Druk doesn't have keywords for int/string types?
  // "primitive types: གྲངས་ཀ (int64)..."
  // Yes, KwNumber, KwString, KwBoolean ARE keywords.
  if (match(TokenKind::KwNumber) || match(TokenKind::KwString) ||
      match(TokenKind::KwBoolean)) {
    return parse_var_declaration();
  }

  return parse_expression_statement();
}

Stmt *Parser::parse_block() {
  // Expect the '{' to already be consumed by the caller
  auto *block = arena_.make<BlockStmt>();
  block->kind = NodeKind::Block;

  std::vector<Stmt *> stmts;
  while (!check(TokenKind::RBrace) && !is_at_end()) {
    Stmt *stmt = parse_statement();
    if (stmt) {
      stmts.push_back(stmt);
    } else {
      // If parse_statement returns null (e.g. error/sync), ensure we advance to
      // avoid infinite loop parse_statement generally shouldn't return null
      // unless it synchronized and found nothing? Actually my parse_statement
      // implementation returns result of parse_expression_statement etc. If
      // those return valid ptrs (even if error inside), we loop. If error
      // happened, we need to synchronize.
      synchronize();
      if (check(TokenKind::RBrace) || is_at_end())
        break;
      // If synchronize didn't consume anything and we are stuck, advance
      // manually But synchronize does advance.
    }
    // Extra safeguard: check if we are stuck on the same token?
    // Rely on synchronize() being correct.
  }
  consume(TokenKind::RBrace, "Expect '}' after block.");

  // Allocate array
  if (stmts.empty()) {
    block->statements = nullptr;
    block->count = 0;
  } else {
    block->count = static_cast<uint32_t>(stmts.size());
    block->statements = arena_.allocate_array<Stmt *>(block->count);
    for (size_t i = 0; i < stmts.size(); ++i) {
      block->statements[i] = stmts[i];
    }
  }

  return block;
}

Stmt *Parser::parse_if_statement() {
  consume(TokenKind::LParen, "Expect '(' after 'if'.");
  Expr *condition = parse_expression();
  consume(TokenKind::RParen, "Expect ')' after if condition.");

  consume(TokenKind::LBrace, "Expect '{' before if body.");
  Stmt *then_branch = parse_block(); // Block required by grammar
  Stmt *else_branch = nullptr;

  if (match(TokenKind::KwElse)) {
    consume(TokenKind::LBrace, "Expect '{' before else body.");
    else_branch = parse_block();
  }

  auto *stmt = arena_.make<IfStmt>();
  stmt->kind = NodeKind::If;
  stmt->condition = condition;
  stmt->then_branch = then_branch;
  stmt->else_branch = else_branch;
  return stmt;
}

Stmt *Parser::parse_expression_statement() {
  Expr *expr = parse_expression();
  consume(TokenKind::Semicolon, "Expect ';' after expression.");
  auto *stmt = arena_.make<ExpressionStmt>();
  stmt->kind = NodeKind::ExpressionStmt;
  stmt->expression = expr;
  return stmt;
}

// Implement loop/return in similar fashion...
Stmt *Parser::parse_loop_statement() {
  consume(TokenKind::LParen, "Expect '(' after 'loop'.");
  Expr *condition = parse_expression();
  consume(TokenKind::RParen, "Expect ')' after loop condition.");

  consume(TokenKind::LBrace, "Expect '{' before loop body.");
  Stmt *body = parse_block(); // Block required by grammar

  auto *stmt = arena_.make<LoopStmt>();
  stmt->kind = NodeKind::Loop;
  stmt->condition = condition;
  stmt->body = body;
  return stmt;
}

Stmt *Parser::parse_return_statement() {
  auto *stmt = arena_.make<ReturnStmt>();
  stmt->kind = NodeKind::Return;
  
  if (!check(TokenKind::Semicolon)) {
    stmt->value = parse_expression();
  } else {
    stmt->value = nullptr;
  }
  
  consume(TokenKind::Semicolon, "Expect ';' after return value.");
  return stmt;
}

} // namespace druk
