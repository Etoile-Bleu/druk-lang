#pragma once
#include <cstdint>
#include <memory>
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
    Value() : type_(ValueType::Nil) { data_.i = 0; }
    explicit Value(int64_t value) : type_(ValueType::Int) { data_.i = value; }
    explicit Value(bool value) : type_(ValueType::Bool) { data_.b = value; }
    explicit Value(std::string_view value) : type_(ValueType::String) {
        data_.s = value;
    }
    explicit Value(std::shared_ptr<ObjFunction> value)
            : type_(ValueType::Function), obj_(std::move(value)) {
        data_.i = 0;
    }
    explicit Value(std::shared_ptr<ObjArray> value)
            : type_(ValueType::Array), obj_(std::move(value)) {
        data_.i = 0;
    }
    explicit Value(std::shared_ptr<ObjStruct> value)
            : type_(ValueType::Struct), obj_(std::move(value)) {
        data_.i = 0;
    }

    [[nodiscard]] ValueType type() const { return type_; }

    [[nodiscard]] bool is_nil() const { return type_ == ValueType::Nil; }
    [[nodiscard]] bool is_int() const { return type_ == ValueType::Int; }
    [[nodiscard]] bool is_bool() const { return type_ == ValueType::Bool; }
    [[nodiscard]] bool is_string() const { return type_ == ValueType::String; }
    [[nodiscard]] bool is_function() const { return type_ == ValueType::Function; }
    [[nodiscard]] bool is_array() const { return type_ == ValueType::Array; }
    [[nodiscard]] bool is_struct() const { return type_ == ValueType::Struct; }

    [[nodiscard]] int64_t as_int() const { return data_.i; }
    [[nodiscard]] bool as_bool() const { return data_.b; }
    [[nodiscard]] std::string_view as_string() const { return data_.s; }

    [[nodiscard]] std::shared_ptr<ObjFunction> as_function() const {
        return std::static_pointer_cast<ObjFunction>(obj_);
    }
    [[nodiscard]] std::shared_ptr<ObjArray> as_array() const {
        return std::static_pointer_cast<ObjArray>(obj_);
    }
    [[nodiscard]] std::shared_ptr<ObjStruct> as_struct() const {
        return std::static_pointer_cast<ObjStruct>(obj_);
    }

    [[nodiscard]] bool operator==(const Value &other) const {
        if (type_ != other.type_) {
            return false;
        }
        switch (type_) {
        case ValueType::Nil:
            return true;
        case ValueType::Int:
            return data_.i == other.data_.i;
        case ValueType::Bool:
            return data_.b == other.data_.b;
        case ValueType::String:
            return data_.s == other.data_.s;
        case ValueType::Function:
        case ValueType::Array:
        case ValueType::Struct:
            return obj_.get() == other.obj_.get();
        default:
            return false;
        }
    }

    [[nodiscard]] bool operator!=(const Value &other) const {
        return !(*this == other);
    }

private:
    ValueType type_{};
    union Data {
        int64_t i;
        bool b;
        std::string_view s;
        Data() : i(0) {}
    } data_;
    std::shared_ptr<void> obj_{};
};

} // namespace druk
