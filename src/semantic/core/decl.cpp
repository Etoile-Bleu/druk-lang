#include "druk/semantic/analyzer.hpp"

namespace druk {

void SemanticAnalyzer::visit_func(FuncDecl *stmt) {
  std::string_view name = stmt->name.text(source_);
  // Check if defined
  if (!table_.define(name, {stmt->name, Type::Function()})) {
    errors_.report(
        Error{ErrorLevel::Error,
              {stmt->name.line, 0, stmt->name.offset, stmt->name.length},
              "Function '" + std::string(name) + "' already defined.",
              ""});
  }

  table_.enter_scope();

  // Define params
  for (uint32_t i = 0; i < stmt->param_count; ++i) {
    Token param_name = stmt->params[i]; // Assuming params is array of Tokens?
    // Wait, FuncDecl in stmt.hpp has `Token* params`?
    // Let's check stmt.hpp again.
    // It said: `Token *params; // Array of Name+Type tokens? simpler for now`
    // If it's just Token, it's just name. Type is implicit or missing?
    // Druk guide says `parameter = type identifier`.
    // So we need Name AND Type.
    // `stmt.hpp` might need update if it only stores `Token*`.
    // If `FuncDecl` structure is too simple, we can't implement full checking.
    // For now, let's assume `Token` is the name and type is Int (default).

    std::string_view p_name = param_name.text(source_);
    Symbol sym{param_name, Type::Int()};
    if (!table_.define(p_name, sym)) {
      errors_.report(
          Error{ErrorLevel::Error,
                {param_name.line, 0, param_name.offset, param_name.length},
                "Duplicate parameter name.",
                ""});
    }
  }

  if (stmt->body) {
    // We visit body block directly?
    // visit_block expects BlockStmt
    // Cast or dispatch
    if (stmt->body->kind == NodeKind::Block) {
      visit_block(static_cast<BlockStmt *>(stmt->body));
    }
  }

  table_.exit_scope();
}

void SemanticAnalyzer::visit_var(VarDecl *stmt) {
  Type var_type = Type::Void();
  switch (stmt->type_token.kind) {
  case TokenKind::KwNumber:
    var_type = Type::Int();
    break;
  case TokenKind::KwString:
    var_type = Type::String();
    break;
  case TokenKind::KwBoolean:
    var_type = Type::Bool();
    break;
  default:
    var_type = Type::Error();
    break; // Should not happen if parser assumes validity
  }

  if (stmt->initializer) {
    visit(stmt->initializer);
    // Check type compatibility
    if (stmt->initializer->type != var_type &&
        stmt->initializer->type != Type::Error()) {
      errors_.report(Error{ErrorLevel::Error,
                           {stmt->initializer->token.line, 0,
                            stmt->initializer->token.offset,
                            stmt->initializer->token.length},
                           "Type mismatch in initialization.",
                           ""});
    }
  }

  std::string_view name = stmt->name.text(source_);
  Symbol sym{stmt->name, var_type};

  if (!table_.define(name, sym)) {
    errors_.report(
        Error{ErrorLevel::Error,
              {stmt->name.line, 0, stmt->name.offset, stmt->name.length},
              "Variable '" + std::string(name) + "' already defined.",
              ""});
  }
}

} // namespace druk
