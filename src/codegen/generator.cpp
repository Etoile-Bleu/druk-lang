#include "druk/codegen/generator.hpp"
#include <iostream>

namespace druk {

Generator::Generator(Chunk &chunk, StringInterner &interner,
                     ErrorReporter &errors, std::string_view source)
    : output_chunk_(chunk), interner_(interner), errors_(errors),
      source_(source) {}

void Generator::init_compiler(Generator::Compiler *compiler, int type) {
  compiler->enclosing = current_compiler_;
  compiler->function = nullptr; // Initialize or create function?
  // Logic from Lox:
  compiler->function = std::make_shared<ObjFunction>();

  if (type == 0) { // TYPE_SCRIPT
    compiler->function->name = "<script>";
  } else {
    compiler->function->name = ""; // Will be set by visit_func
  }

  current_compiler_ = compiler;

  // Reserve slot 0 for local 'this' or script
  Local local;
  local.depth = 0;
  local.name = ""; // Empty name
  current_compiler_->locals.push_back(local);
}

bool Generator::generate(const std::vector<Stmt *> &statements) {
  Compiler root;
  init_compiler(&root, 0);
  root_function_ = root.function;

  for (Stmt *stmt : statements) {
    if (stmt)
      visit(stmt);
  }
  emit_opcode(OpCode::Return);

  // Copy to output chunk from root compiler
  output_chunk_ = current_compiler_->function->chunk;

  end_compiler();

  return !errors_.has_errors();
}

// Emitting helpers
void Generator::emit_byte(uint8_t byte) { current_chunk().write(byte, line_); }

void Generator::emit_opcode(OpCode opcode) {
  current_chunk().write(opcode, line_);
}

void Generator::emit_constant(Value value) {
  int index = current_chunk().add_constant(value);
  if (index > 255) {
    // Emit OP_CONSTANT_LONG if needed, for now assume < 256
    // errors_.report(...)
  }
  emit_opcode(OpCode::Constant);
  emit_byte(static_cast<uint8_t>(index));
}

void Generator::emit_loop(int loop_start) {
  emit_opcode(OpCode::Loop);

  int offset = static_cast<int>(current_chunk().code().size()) - loop_start + 2;
  if (offset > UINT16_MAX)
    errors_.report(Error{ErrorLevel::Error, {}, "Loop body too large.", ""});

  emit_byte(static_cast<uint8_t>((offset >> 8) & 0xff));
  emit_byte(static_cast<uint8_t>(offset & 0xff));
}

int Generator::emit_jump(OpCode opcode) {
  emit_opcode(opcode);
  emit_byte(0xff); // Placeholder 1
  emit_byte(0xff); // Placeholder 2
  return static_cast<int>(current_chunk().code().size()) - 2;
}

void Generator::patch_jump(int offset) {
  int jump = static_cast<int>(current_chunk().code().size()) - offset - 2;

  if (jump > UINT16_MAX) {
    errors_.report(
        Error{ErrorLevel::Error, {}, "Too much code to jump over.", ""});
  }

  current_chunk().patch(static_cast<size_t>(offset),
                        static_cast<uint8_t>((jump >> 8) & 0xff));
  current_chunk().patch(static_cast<size_t>(offset) + 1,
                        static_cast<uint8_t>(jump & 0xff));
}

// Local variable helpers
void Generator::begin_scope() { current_compiler_->scope_depth++; }

void Generator::end_scope() {
  current_compiler_->scope_depth--;
  // Pop locals from stack
  while (!current_compiler_->locals.empty() &&
         current_compiler_->locals.back().depth >
             current_compiler_->scope_depth) {
    emit_opcode(OpCode::Pop);
    current_compiler_->locals.pop_back();
  }
}

void Generator::add_local(std::string_view name) {
  if (current_compiler_->locals.size() == 256) { // Max locals
    // Assuming we don't have a token here, using empty location
    errors_.report(Error{ErrorLevel::Error,
                         {0, 0, 0, 0},
                         "Too many local variables in function.",
                         ""});
    return;
  }
  current_compiler_->locals.push_back(
      Local{name, current_compiler_->scope_depth});
}

int Generator::resolve_local(std::string_view name) {
  for (int i = static_cast<int>(current_compiler_->locals.size()) - 1; i >= 0;
       i--) {
    if (current_compiler_->locals[static_cast<size_t>(i)].name == name) {
      return i;
    }
  }
  return -1;
}

void Generator::end_compiler() {
  current_compiler_ = current_compiler_->enclosing;
}

Chunk &Generator::current_chunk() { return current_compiler_->function->chunk; }

} // namespace druk
