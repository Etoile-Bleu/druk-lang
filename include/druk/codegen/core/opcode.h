#pragma once
#include <cstdint>

namespace druk {

enum class OpCode : uint8_t {
  Return,
  Constant,
  Nil,
  True,
  False,
  Pop,
  GetLocal,
  SetLocal,
  GetGlobal,
  DefineGlobal,
  SetGlobal,
  Equal,
  Greater,
  Less,
  Add,
  Subtract,
  Multiply,
  Divide,
  Not,
  Negate,
  Print,
  Jump,
  JumpIfFalse,
  Loop,
  Call,
  
  // Collections
  BuildArray,    // Build array from N stack values
  Index,         // Get array[index] or struct.field
  IndexSet,      // Set array[index] = value
  BuildStruct,   // Build struct from N field values
  GetField,      // Get struct.field by name
  SetField,      // Set struct.field = value
  
  // Built-in functions
  Len,           // Get length of array or struct (field count)
  Push,          // Push element to array
  PopArray,      // Pop element from array and return it
  TypeOf,        // Get type name as string
  Keys,          // Get struct keys as array
  Values,        // Get struct values as array
  Contains,      // Check if array contains value or struct has key
  Input,         // Read a line from stdin
};

} // namespace druk
