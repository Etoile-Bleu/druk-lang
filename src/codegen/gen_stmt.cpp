#include "druk/codegen/generator.hpp"
#include <iostream>

namespace druk {

void Generator::visit(Stmt *stmt) {
  if (!stmt)
    return;
  // Update line info if possible from stmt->token
  switch (stmt->kind) {
  case NodeKind::Block:
    visit_block(static_cast<BlockStmt *>(stmt));
    break;
  case NodeKind::If:
    visit_if(static_cast<IfStmt *>(stmt));
    break;
  case NodeKind::Loop:
    visit_loop(static_cast<LoopStmt *>(stmt));
    break;
  case NodeKind::Return:
    visit_return(static_cast<ReturnStmt *>(stmt));
    break;
  case NodeKind::Print:
    visit_print(static_cast<PrintStmt *>(stmt));
    break;
  case NodeKind::ExpressionStmt:
    visit_expr_stmt(static_cast<ExpressionStmt *>(stmt));
    break;
  case NodeKind::Variable:
    visit_var(static_cast<VarDecl *>(stmt));
    break;
  case NodeKind::Function:
    visit_func(static_cast<FuncDecl *>(stmt));
    break;
  default:
    break;
  }
}

void Generator::visit_print(PrintStmt *stmt) {
  visit(stmt->expression);
  emit_opcode(OpCode::Print);
}

void Generator::visit_block(BlockStmt *stmt) {
  begin_scope();
  if (stmt->statements) {
    for (uint32_t i = 0; i < stmt->count; ++i) {
      visit(stmt->statements[i]);
    }
  }
  end_scope();
}

void Generator::visit_expr_stmt(ExpressionStmt *stmt) {
  visit(stmt->expression);
  emit_opcode(OpCode::Pop); // Pop result of expression statement
}

void Generator::visit_var(VarDecl *stmt) {
  if (stmt->initializer) {
    visit(stmt->initializer);
  } else {
    // Default init? Or error?
    // For now push 0 / null
    emit_constant(Value(int64_t{0}));
  }

  // For global variables:
  // DefineGlobal name
  std::string_view name = stmt->name.text(source_);

  if (current_compiler_->scope_depth > 0) {
    // Local variable
    // Ensure not redeclared in same scope?
    for (int i = static_cast<int>(current_compiler_->locals.size()) - 1; i >= 0;
         i--) {
      if (current_compiler_->locals[static_cast<size_t>(i)].depth <
          current_compiler_->scope_depth)
        break;
      if (current_compiler_->locals[static_cast<size_t>(i)].name == name) {
        errors_.report(
            Error{ErrorLevel::Error,
                  {stmt->name.line, 0, stmt->name.offset, stmt->name.length},
                  "Variable with this name already declared in this scope.",
                  ""});
      }
    }
    add_local(name);
    // No opcode needed, value is already on stack (from initializer)
  } else {
    // Global variable
    uint8_t arg = current_chunk().add_constant(Value(name));
    emit_opcode(OpCode::DefineGlobal);
    emit_byte(arg);
  }
}

void Generator::visit_if(IfStmt *stmt) {
  visit(stmt->condition);
  int then_jump = emit_jump(OpCode::JumpIfFalse);
  emit_opcode(OpCode::Pop); // Pop condition (true case)

  visit(stmt->then_branch);

  int else_jump = emit_jump(OpCode::Jump);
  patch_jump(then_jump);

  emit_opcode(OpCode::Pop); // Pop condition (false case)

  if (stmt->else_branch) {
    visit(stmt->else_branch);
  }
  patch_jump(else_jump);
}

void Generator::visit_loop(LoopStmt *stmt) {
  int loop_start = static_cast<int>(current_chunk().code().size());

  visit(stmt->condition);
  int exit_jump = emit_jump(OpCode::JumpIfFalse);
  emit_opcode(OpCode::Pop); // Pop condition (true case)

  visit(stmt->body);
  emit_loop(loop_start);

  patch_jump(exit_jump);
  emit_opcode(OpCode::Pop); // Pop condition (false case)
}

void Generator::visit_return(ReturnStmt *stmt) {
  if (stmt->value) {
    visit(stmt->value);
    emit_opcode(OpCode::Return);
  } else {
    // Return nil/void?
    emit_constant(Value(int64_t{0})); // Void placeholder
    emit_opcode(OpCode::Return);
  }
}

void Generator::visit_func(FuncDecl *stmt) {
  std::string_view name = stmt->name.text(source_);

  Compiler compiler;
  init_compiler(&compiler, 1); // TYPE_FUNCTION
  current_compiler_->function->name = std::string(name);
  current_compiler_->function->arity = static_cast<int>(stmt->param_count);

  begin_scope();

  for (uint32_t i = 0; i < stmt->param_count; ++i) {
    // Parameters are just local variables at the start of the function
    std::string_view param_name = stmt->params[i].text(source_);
    add_local(param_name);
  }

  visit(stmt->body);

  // Function always returns 0 (void) if it hasn't already.
  emit_constant(Value(int64_t{0}));
  emit_opcode(OpCode::Return);

  auto func = current_compiler_->function;
  end_compiler();

  // Push the function onto the stack as a constant
  emit_constant(Value(func));

  // Define the variable that holds the function
  if (current_compiler_->scope_depth > 0) {
    add_local(name);
  } else {
    uint8_t arg = current_chunk().add_constant(Value(name));
    emit_opcode(OpCode::DefineGlobal);
    emit_byte(arg);
  }
}

} // namespace druk
