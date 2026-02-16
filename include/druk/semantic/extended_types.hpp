// Extended type system for collections
#pragma once

#include "druk/semantic/types.hpp"
#include <memory>
#include <vector>
#include <unordered_map>

namespace druk {

// Array type: Array<element_type>
struct ArrayType {
  Type element_type;
  
  bool operator==(const ArrayType &other) const {
    return element_type == other.element_type;
  }
};

// Struct field
struct StructField {
  std::string name;
  Type type;
};

// Struct type: { field1: type1, field2: type2 }
struct StructType {
  std::vector<StructField> fields;
  
  // Lookup field by name
  const StructField* find_field(const std::string& name) const {
    for (const auto& field : fields) {
      if (field.name == name) {
        return &field;
      }
    }
    return nullptr;
  }
  
  bool operator==(const StructType &other) const {
    if (fields.size() != other.fields.size()) return false;
    for (size_t i = 0; i < fields.size(); i++) {
      if (fields[i].name != other.fields[i].name ||
          fields[i].type != other.fields[i].type) {
        return false;
      }
    }
    return true;
  }
};

// Extended type that can be primitive or collection
struct ExtendedType {
  TypeKind kind;
  
  // Optional: for complex types
  std::shared_ptr<ArrayType> array_type;
  std::shared_ptr<StructType> struct_type;
  
  static ExtendedType Array(Type element) {
    ExtendedType t;
    t.kind = TypeKind::Error; // Use Error as placeholder for now
    t.array_type = std::make_shared<ArrayType>(ArrayType{element});
    return t;
  }
  
  static ExtendedType Struct(std::vector<StructField> fields) {
    ExtendedType t;
    t.kind = TypeKind::Error; // Use Error as placeholder
    t.struct_type = std::make_shared<StructType>(StructType{fields});
    return t;
  }
  
  static ExtendedType FromSimple(Type t) {
    ExtendedType ext;
    ext.kind = t.kind;
    return ext;
  }
  
  bool is_array() const { return array_type != nullptr; }
  bool is_struct() const { return struct_type != nullptr; }
};

} // namespace druk
