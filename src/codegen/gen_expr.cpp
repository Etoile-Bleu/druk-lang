#include "druk/codegen/generator.hpp"
#include <iostream>
#include <string>

namespace druk {

void Generator::visit(Expr *expr) {
  if (!expr)
    return;
  switch (expr->kind) {
  case NodeKind::Binary:
    visit_binary(static_cast<BinaryExpr *>(expr));
    break;
  case NodeKind::Unary:
    visit_unary(static_cast<UnaryExpr *>(expr));
    break;
  case NodeKind::Literal:
    visit_literal(static_cast<LiteralExpr *>(expr));
    break;
  case NodeKind::VariableExpr:
    visit_variable(static_cast<VariableExpr *>(expr));
    break;
  case NodeKind::Grouping:
    visit_grouping(static_cast<GroupingExpr *>(expr));
    break;
  case NodeKind::Assignment:
    visit_assign(static_cast<AssignmentExpr *>(expr));
    break;
  case NodeKind::Call:
    visit_call(static_cast<CallExpr *>(expr));
    break;
  case NodeKind::Logical:
    visit_logical(static_cast<LogicalExpr *>(expr));
    break;
  case NodeKind::ArrayLiteral:
    visit_array_literal(static_cast<ArrayLiteralExpr *>(expr));
    break;
  case NodeKind::Index:
    visit_index(static_cast<IndexExpr *>(expr));
    break;
  case NodeKind::StructLiteral:
    visit_struct_literal(static_cast<StructLiteralExpr *>(expr));
    break;
  case NodeKind::MemberAccess:
    visit_member_access(static_cast<MemberAccessExpr *>(expr));
    break;
  default:
    break;
  }
}

void Generator::visit_literal(LiteralExpr *expr) {
  if (expr->literal_value.is_int()) {
    emit_constant(Value(expr->literal_value.as_int()));
  } else if (expr->literal_value.is_string()) {
    emit_constant(Value(expr->literal_value.as_string()));
  } else if (expr->literal_value.is_bool()) {
    emit_opcode(expr->literal_value.as_bool() ? OpCode::True : OpCode::False);
  } else if (expr->literal_value.is_nil()) {
    emit_opcode(OpCode::Nil);
  }
}

void Generator::visit_grouping(GroupingExpr *expr) { visit(expr->expression); }

void Generator::visit_assign(AssignmentExpr *expr) {
  // Check what kind of target we have
  if (expr->target->kind == NodeKind::VariableExpr) {
    // Simple variable: x = value
    visit(expr->value);
    
    auto *var = static_cast<VariableExpr *>(expr->target);
    std::string_view name = var->name.text(source_);
    int arg = resolve_local(name);

    if (arg != -1) {
      emit_opcode(OpCode::SetLocal);
      emit_byte(static_cast<uint8_t>(arg));
    } else {
      uint8_t arg_idx = current_chunk().add_constant(Value(name));
      emit_opcode(OpCode::SetGlobal);
      emit_byte(arg_idx);
    }
  } else if (expr->target->kind == NodeKind::Index) {
    // Array indexing: arr[i] = value
    visit_index_set(static_cast<IndexExpr *>(expr->target), expr->value);
  } else if (expr->target->kind == NodeKind::MemberAccess) {
    // Struct field: obj.field = value
    visit_member_set(static_cast<MemberAccessExpr *>(expr->target), expr->value);
  }
}

void Generator::visit_binary(BinaryExpr *expr) {
  visit(expr->left);
  visit(expr->right);
  // Use token from Node base, not member 'op'
  switch (expr->token.kind) {
  case TokenKind::Plus:
    emit_opcode(OpCode::Add);
    break;
  case TokenKind::Minus:
    emit_opcode(OpCode::Subtract);
    break;
  case TokenKind::Star:
    emit_opcode(OpCode::Multiply);
    break;
  case TokenKind::Slash:
    emit_opcode(OpCode::Divide);
    break;

  case TokenKind::EqualEqual:
    emit_opcode(OpCode::Equal);
    break;
  case TokenKind::BangEqual:
    emit_opcode(OpCode::Equal);
    emit_opcode(OpCode::Not);
    break;
  case TokenKind::Greater:
    emit_opcode(OpCode::Greater);
    break;
  case TokenKind::GreaterEqual:
    emit_opcode(OpCode::Less);
    emit_opcode(OpCode::Not);
    break;
  case TokenKind::Less:
    emit_opcode(OpCode::Less);
    break;
  case TokenKind::LessEqual:
    emit_opcode(OpCode::Greater);
    emit_opcode(OpCode::Not);
    break;

  case TokenKind::And:
    break;
  default:
    break;
  }
}

void Generator::visit_unary(UnaryExpr *expr) {
  visit(expr->right);
  switch (expr->token.kind) {
  case TokenKind::Minus:
    emit_opcode(OpCode::Negate);
    break;
  case TokenKind::Bang:
    emit_opcode(OpCode::Not);
    break;
  default:
    break;
  }
}

void Generator::visit_variable(VariableExpr *expr) {
  std::string_view name = expr->name.text(source_);

  int arg = resolve_local(name);
  if (arg != -1) {
    emit_opcode(OpCode::GetLocal);
    emit_byte(static_cast<uint8_t>(arg));
  } else {
    uint8_t arg_idx = current_chunk().add_constant(Value(name));
    emit_opcode(OpCode::GetGlobal);
    emit_byte(arg_idx);
  }
}

void Generator::visit_call(CallExpr *expr) {
  // Check if it's a built-in function
  if (expr->callee->kind == NodeKind::VariableExpr) {
    auto *var = static_cast<VariableExpr *>(expr->callee);
    std::string_view name = var->name.text(source_);
    
    if (name == "ཚད་" && expr->arg_count == 1) {
      visit(static_cast<Expr *>(expr->args[0]));
      emit_opcode(OpCode::Len);
      return;
    } else if (name == "སྣོན་" && expr->arg_count == 2) {
      visit(static_cast<Expr *>(expr->args[0])); // array
      visit(static_cast<Expr *>(expr->args[1])); // element
      emit_opcode(OpCode::Push);
      return;
    } else if (name == "ཕྱིར་ལེན་" && expr->arg_count == 1) {
      visit(static_cast<Expr *>(expr->args[0]));
      emit_opcode(OpCode::PopArray);
      return;
    } else if (name == "རིགས་" && expr->arg_count == 1) {
      visit(static_cast<Expr *>(expr->args[0]));
      emit_opcode(OpCode::TypeOf);
      return;
    } else if (name == "མིང་ཐོ་" && expr->arg_count == 1) {
      visit(static_cast<Expr *>(expr->args[0]));
      emit_opcode(OpCode::Keys);
      return;
    } else if (name == "བེད་སྤྱོད་" && expr->arg_count == 1) {
      visit(static_cast<Expr *>(expr->args[0]));
      emit_opcode(OpCode::Values);
      return;
    } else if (name == "ཡོད་" && expr->arg_count == 2) {
      visit(static_cast<Expr *>(expr->args[0])); // container
      visit(static_cast<Expr *>(expr->args[1])); // value/key
      emit_opcode(OpCode::Contains);
      return;
    } else if (name == "ནང་འཇུག་" && expr->arg_count == 0) {
      emit_opcode(OpCode::Input);
      return;
    }
  }
  
  // Regular function call
  visit(expr->callee);
  for (uint32_t i = 0; i < expr->arg_count; ++i) {
    visit(static_cast<Expr *>(expr->args[i]));
  }
  emit_opcode(OpCode::Call);
  emit_byte(static_cast<uint8_t>(expr->arg_count));
}

void Generator::visit_logical(LogicalExpr *expr) {
  // Short-circuit evaluation for && and ||
  visit(expr->left);

  if (expr->token.kind == TokenKind::And) {
    // For AND: if left is false, skip right and keep false
    // left
    // JUMP_IF_FALSE -> end (skip right)
    // POP (left was true, evaluate right)
    // right
    // end:
    size_t end_jump = emit_jump(OpCode::JumpIfFalse);
    emit_opcode(OpCode::Pop); // Pop left if it was true
    visit(expr->right);
    patch_jump(end_jump);
  } else if (expr->token.kind == TokenKind::Or) {
    // For OR: if left is true, skip right and keep true
    // This is trickier. We need to invert the logic:
    // left
    // JUMP_IF_FALSE -> evaluate_right
    // JUMP -> end (left was true, skip right)
    // evaluate_right:
    // POP (left was false, evaluate right)
    // right
    // end:
    size_t else_jump = emit_jump(OpCode::JumpIfFalse);
    size_t end_jump = emit_jump(OpCode::Jump);
    patch_jump(else_jump);
    emit_opcode(OpCode::Pop);
    visit(expr->right);
    patch_jump(end_jump);
  }
}

} // namespace druk
