#pragma once

#include <cstdint>
#include <string>

namespace druk {

enum class TypeKind { Void, Int, String, Bool, Function, Error };

struct Type {
  TypeKind kind;
  // For function types, we might need return type and param types.
  // Kept simple for now (100 LOC limit).
  // Pointers for complex types management?

  bool operator==(const Type &other) const { return kind == other.kind; }

  bool operator!=(const Type &other) const { return kind != other.kind; }

  static Type Int() { return {TypeKind::Int}; }
  static Type String() { return {TypeKind::String}; }
  static Type Bool() { return {TypeKind::Bool}; }
  static Type Void() { return {TypeKind::Void}; }
  static Type Function() { return {TypeKind::Function}; }
  static Type Error() { return {TypeKind::Error}; }
};

std::string type_to_string(Type type);

} // namespace druk
