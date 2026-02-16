// Code generation for arrays and structs
#include "druk/codegen/generator.hpp"
#include "druk/codegen/obj.hpp"

namespace druk {

void Generator::visit_array_literal(ArrayLiteralExpr *expr) {
  // Push all elements onto stack
  for (uint32_t i = 0; i < expr->count; i++) {
    visit(expr->elements[i]);
  }
  
  // Build array from N stack values
  emit_opcode(OpCode::BuildArray);
  emit_byte(static_cast<uint8_t>(expr->count));
}

void Generator::visit_index(IndexExpr *expr) {
  // Push array/struct
  visit(expr->array);
  
  // Push index
  visit(expr->index);
  
  // Get element
  emit_opcode(OpCode::Index);
}

void Generator::visit_index_set(IndexExpr *lhs, Expr *rhs) {
  // For assignment: array[index] = value
  
  // Push array
  visit(lhs->array);
  
  // Push index
  visit(lhs->index);
  
  // Push value
  visit(rhs);
  
  // Set element
  emit_opcode(OpCode::IndexSet);
}

void Generator::visit_struct_literal(StructLiteralExpr *expr) {
  // Push field names as constants, then values
  for (uint32_t i = 0; i < expr->field_count; i++) {
    // Push field name
    std::string_view name = expr->field_names[i].text(source_);
    emit_constant(Value(name));
    
    // Push field value
    visit(expr->field_values[i]);
  }
  
  // Build struct from N field pairs
  emit_opcode(OpCode::BuildStruct);
  emit_byte(static_cast<uint8_t>(expr->field_count));
}

void Generator::visit_member_access(MemberAccessExpr *expr) {
  // Push object
  visit(expr->object);
  
  // Push field name
  std::string_view name = expr->member_name.text(source_);
  uint8_t name_idx = current_chunk().add_constant(Value(name));
  
  // Get field
  emit_opcode(OpCode::GetField);
  emit_byte(name_idx);
}

void Generator::visit_member_set(MemberAccessExpr *lhs, Expr *rhs) {
  // For assignment: obj.field = value
  
  // Push object
  visit(lhs->object);
  
  // Push value
  visit(rhs);
  
  // Push field name
  std::string_view name = lhs->member_name.text(source_);
  uint8_t name_idx = current_chunk().add_constant(Value(name));
  
  // Set field
  emit_opcode(OpCode::SetField);
  emit_byte(name_idx);
}

} // namespace druk
