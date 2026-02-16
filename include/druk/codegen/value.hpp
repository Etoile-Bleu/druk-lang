#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <new>
#include <string>
#include <string_view>

namespace druk {

// Forward declarations
struct ObjFunction;
struct ObjArray;
struct ObjStruct;

enum class ValueType : uint8_t {
  Nil,
  Int,
  Bool,
  String,
  Function,
  Array,
  Struct,
};

class Value {
public:
  Value() : type_(ValueType::Nil) { as.i = 0; }

  explicit Value(int64_t value) : type_(ValueType::Int) { as.i = value; }
  explicit Value(bool value) : type_(ValueType::Bool) { as.b = value; }
  explicit Value(std::string_view value) : type_(ValueType::String) {
    as.s = value;
  }

  explicit Value(std::shared_ptr<ObjFunction> value)
      : type_(ValueType::Function) {
    new (&as.function) std::shared_ptr<ObjFunction>(std::move(value));
  }

  explicit Value(std::shared_ptr<ObjArray> value) : type_(ValueType::Array) {
    new (&as.array) std::shared_ptr<ObjArray>(std::move(value));
  }

  explicit Value(std::shared_ptr<ObjStruct> value) : type_(ValueType::Struct) {
    new (&as.structure) std::shared_ptr<ObjStruct>(std::move(value));
  }

  // Copy Constructor
  Value(const Value &other) : type_(other.type_) {
    switch (type_) {
    case ValueType::Nil:
    case ValueType::Int:
      as.i = other.as.i;
      break;
    case ValueType::Bool:
      as.b = other.as.b;
      break;
    case ValueType::String:
      as.s = other.as.s;
      break;
    case ValueType::Function:
      new (&as.function) std::shared_ptr<ObjFunction>(other.as.function);
      break;
    case ValueType::Array:
      new (&as.array) std::shared_ptr<ObjArray>(other.as.array);
      break;
    case ValueType::Struct:
      new (&as.structure) std::shared_ptr<ObjStruct>(other.as.structure);
      break;
    }
  }

  // Move Constructor
  Value(Value &&other) noexcept : type_(other.type_) {
    switch (type_) {
    case ValueType::Nil:
    case ValueType::Int:
      as.i = other.as.i;
      break;
    case ValueType::Bool:
      as.b = other.as.b;
      break;
    case ValueType::String:
      as.s = other.as.s;
      break;
    case ValueType::Function:
      new (&as.function)
          std::shared_ptr<ObjFunction>(std::move(other.as.function));
      break;
    case ValueType::Array:
      new (&as.array) std::shared_ptr<ObjArray>(std::move(other.as.array));
      break;
    case ValueType::Struct:
      new (&as.structure)
          std::shared_ptr<ObjStruct>(std::move(other.as.structure));
      break;
    }
    // No need to reset other.type_ to Nil, but it's good practice if we want
    // 'other' to be valid-but-unspecified. However, standard move semantics
    // allow leaving it as is, provided destructor handles it. But since we
    // MOVED the shared_ptr, it is null now.
  }

  // Assignment Operator
  Value &operator=(const Value &other) {
    if (this == &other)
      return *this;

    if (type_ == other.type_) {
      switch (type_) {
      case ValueType::Nil:
      case ValueType::Int:
        as.i = other.as.i;
        return *this;
      case ValueType::Bool:
        as.b = other.as.b;
        return *this;
      case ValueType::String:
        as.s = other.as.s;
        return *this;
      case ValueType::Function:
        as.function = other.as.function;
        return *this;
      case ValueType::Array:
        as.array = other.as.array;
        return *this;
      case ValueType::Struct:
        as.structure = other.as.structure;
        return *this;
      }
    }

    // Destruct current
    this->~Value();

    // Construct new
    new (this) Value(other);

    return *this;
  }

  // Move Assignment
  Value &operator=(Value &&other) noexcept {
    if (this == &other)
      return *this;

    if (type_ == other.type_) {
      switch (type_) {
      case ValueType::Nil:
      case ValueType::Int:
        as.i = other.as.i;
        return *this;
      case ValueType::Bool:
        as.b = other.as.b;
        return *this;
      case ValueType::String:
        as.s = other.as.s;
        return *this;
      case ValueType::Function:
        as.function = std::move(other.as.function);
        return *this;
      case ValueType::Array:
        as.array = std::move(other.as.array);
        return *this;
      case ValueType::Struct:
        as.structure = std::move(other.as.structure);
        return *this;
      }
    }

    // Destruct current
    this->~Value();

    // Construct new
    new (this) Value(std::move(other));

    return *this;
  }

  ~Value() {
    switch (type_) {
    case ValueType::Function:
      as.function.~shared_ptr();
      break;
    case ValueType::Array:
      as.array.~shared_ptr();
      break;
    case ValueType::Struct:
      as.structure.~shared_ptr();
      break;
    default:
      break;
    }
  }

  [[nodiscard]] ValueType type() const { return type_; }

  [[nodiscard]] bool is_nil() const { return type_ == ValueType::Nil; }
  [[nodiscard]] bool is_int() const { return type_ == ValueType::Int; }
  [[nodiscard]] bool is_bool() const { return type_ == ValueType::Bool; }
  [[nodiscard]] bool is_string() const { return type_ == ValueType::String; }
  [[nodiscard]] bool is_function() const {
    return type_ == ValueType::Function;
  }
  [[nodiscard]] bool is_array() const { return type_ == ValueType::Array; }
  [[nodiscard]] bool is_struct() const { return type_ == ValueType::Struct; }

  [[nodiscard]] int64_t as_int() const { return as.i; }
  [[nodiscard]] bool as_bool() const { return as.b; }
  [[nodiscard]] std::string_view as_string() const { return as.s; }

  [[nodiscard]] std::shared_ptr<ObjFunction> as_function() const {
    assert(type_ == ValueType::Function);
    return as.function;
  }
  [[nodiscard]] std::shared_ptr<ObjArray> as_array() const {
    assert(type_ == ValueType::Array);
    return as.array;
  }
  [[nodiscard]] std::shared_ptr<ObjStruct> as_struct() const {
    assert(type_ == ValueType::Struct);
    return as.structure;
  }

  [[nodiscard]] bool operator==(const Value &other) const {
    if (type_ != other.type_) {
      return false;
    }
    switch (type_) {
    case ValueType::Nil:
      return true;
    case ValueType::Int:
      return as.i == other.as.i;
    case ValueType::Bool:
      return as.b == other.as.b;
    case ValueType::String:
      return as.s == other.as.s;
    case ValueType::Function:
      return as.function == other.as.function;
    case ValueType::Array:
      return as.array == other.as.array;
    case ValueType::Struct:
      return as.structure == other.as.structure;
    default:
      return false;
    }
  }

  [[nodiscard]] bool operator!=(const Value &other) const {
    return !(*this == other);
  }

private:
  ValueType type_;
  union UnionData {
    int64_t i;
    bool b;
    std::string_view s;
    std::shared_ptr<ObjFunction> function;
    std::shared_ptr<ObjArray> array;
    std::shared_ptr<ObjStruct> structure;

    UnionData() { i = 0; }
    ~UnionData() {} // handled by Value destructor
  } as;
};

} // namespace druk
