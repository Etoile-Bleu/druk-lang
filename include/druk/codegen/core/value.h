#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <new>
#include <string>
#include <string_view>

namespace druk::codegen
{

// Forward declarations
struct ObjFunction;
struct ObjArray;
struct ObjStruct;

/**
 * @brief Categorizes the different types of values that can exist at runtime.
 */
enum class ValueType : uint8_t
{
    Nil,
    Int,
    Bool,
    String,
    Function,
    Array,
    Struct,
};

/**
 * @brief Represents a value in the Druk language runtime.
 * Implements a tagged union with shared pointer management for objects.
 */
class Value
{
   public:
    Value() : type_(ValueType::Nil)
    {
        data_.i = 0;
    }

    explicit Value(int64_t value) : type_(ValueType::Int)
    {
        data_.i = value;
    }
    explicit Value(bool value) : type_(ValueType::Bool)
    {
        data_.b = value;
    }
    explicit Value(std::string_view value) : type_(ValueType::String)
    {
        data_.s = value;
    }

    explicit Value(std::shared_ptr<ObjFunction> value) : type_(ValueType::Function)
    {
        new (&data_.function) std::shared_ptr<ObjFunction>(std::move(value));
    }

    explicit Value(std::shared_ptr<ObjArray> value) : type_(ValueType::Array)
    {
        new (&data_.array) std::shared_ptr<ObjArray>(std::move(value));
    }

    explicit Value(std::shared_ptr<ObjStruct> value) : type_(ValueType::Struct)
    {
        new (&data_.structure) std::shared_ptr<ObjStruct>(std::move(value));
    }

    // Copy Constructor
    Value(const Value& other) : type_(other.type_)
    {
        switch (type_)
        {
            case ValueType::Nil:
            case ValueType::Int:
                data_.i = other.data_.i;
                break;
            case ValueType::Bool:
                data_.b = other.data_.b;
                break;
            case ValueType::String:
                data_.s = other.data_.s;
                break;
            case ValueType::Function:
                new (&data_.function) std::shared_ptr<ObjFunction>(other.data_.function);
                break;
            case ValueType::Array:
                new (&data_.array) std::shared_ptr<ObjArray>(other.data_.array);
                break;
            case ValueType::Struct:
                new (&data_.structure) std::shared_ptr<ObjStruct>(other.data_.structure);
                break;
        }
    }

    // Move Constructor
    Value(Value&& other) noexcept : type_(other.type_)
    {
        switch (type_)
        {
            case ValueType::Nil:
            case ValueType::Int:
                data_.i = other.data_.i;
                break;
            case ValueType::Bool:
                data_.b = other.data_.b;
                break;
            case ValueType::String:
                data_.s = other.data_.s;
                break;
            case ValueType::Function:
                new (&data_.function) std::shared_ptr<ObjFunction>(std::move(other.data_.function));
                break;
            case ValueType::Array:
                new (&data_.array) std::shared_ptr<ObjArray>(std::move(other.data_.array));
                break;
            case ValueType::Struct:
                new (&data_.structure) std::shared_ptr<ObjStruct>(std::move(other.data_.structure));
                break;
        }
    }

    // Assignment Operator
    Value& operator=(const Value& other)
    {
        if (this == &other)
            return *this;

        if (type_ == other.type_)
        {
            switch (type_)
            {
                case ValueType::Nil:
                case ValueType::Int:
                    data_.i = other.data_.i;
                    return *this;
                case ValueType::Bool:
                    data_.b = other.data_.b;
                    return *this;
                case ValueType::String:
                    data_.s = other.data_.s;
                    return *this;
                case ValueType::Function:
                    data_.function = other.data_.function;
                    return *this;
                case ValueType::Array:
                    data_.array = other.data_.array;
                    return *this;
                case ValueType::Struct:
                    data_.structure = other.data_.structure;
                    return *this;
            }
        }

        this->~Value();
        new (this) Value(other);
        return *this;
    }

    // Move Assignment
    Value& operator=(Value&& other) noexcept
    {
        if (this == &other)
            return *this;

        if (type_ == other.type_)
        {
            switch (type_)
            {
                case ValueType::Nil:
                case ValueType::Int:
                    data_.i = other.data_.i;
                    return *this;
                case ValueType::Bool:
                    data_.b = other.data_.b;
                    return *this;
                case ValueType::String:
                    data_.s = other.data_.s;
                    return *this;
                case ValueType::Function:
                    data_.function = std::move(other.data_.function);
                    return *this;
                case ValueType::Array:
                    data_.array = std::move(other.data_.array);
                    return *this;
                case ValueType::Struct:
                    data_.structure = std::move(other.data_.structure);
                    return *this;
            }
        }

        this->~Value();
        new (this) Value(std::move(other));
        return *this;
    }

    ~Value()
    {
        switch (type_)
        {
            case ValueType::Function:
                data_.function.~shared_ptr();
                break;
            case ValueType::Array:
                data_.array.~shared_ptr();
                break;
            case ValueType::Struct:
                data_.structure.~shared_ptr();
                break;
            default:
                break;
        }
    }

    [[nodiscard]] ValueType type() const
    {
        return type_;
    }

    [[nodiscard]] bool isNil() const
    {
        return type_ == ValueType::Nil;
    }
    [[nodiscard]] bool isInt() const
    {
        return type_ == ValueType::Int;
    }
    [[nodiscard]] bool isBool() const
    {
        return type_ == ValueType::Bool;
    }
    [[nodiscard]] bool isString() const
    {
        return type_ == ValueType::String;
    }
    [[nodiscard]] bool isFunction() const
    {
        return type_ == ValueType::Function;
    }
    [[nodiscard]] bool isArray() const
    {
        return type_ == ValueType::Array;
    }
    [[nodiscard]] bool isStruct() const
    {
        return type_ == ValueType::Struct;
    }

    [[nodiscard]] int64_t asInt() const
    {
        return data_.i;
    }
    [[nodiscard]] bool asBool() const
    {
        return data_.b;
    }
    [[nodiscard]] std::string_view asString() const
    {
        return data_.s;
    }

    [[nodiscard]] std::shared_ptr<ObjFunction> asFunction() const
    {
        assert(type_ == ValueType::Function);
        return data_.function;
    }
    [[nodiscard]] std::shared_ptr<ObjArray> asArray() const
    {
        assert(type_ == ValueType::Array);
        return data_.array;
    }
    [[nodiscard]] std::shared_ptr<ObjStruct> asStruct() const
    {
        assert(type_ == ValueType::Struct);
        return data_.structure;
    }

    [[nodiscard]] bool operator==(const Value& other) const
    {
        if (type_ != other.type_)
        {
            return false;
        }
        switch (type_)
        {
            case ValueType::Nil:
                return true;
            case ValueType::Int:
                return data_.i == other.data_.i;
            case ValueType::Bool:
                return data_.b == other.data_.b;
            case ValueType::String:
                return data_.s == other.data_.s;
            case ValueType::Function:
                return data_.function == other.data_.function;
            case ValueType::Array:
                return data_.array == other.data_.array;
            case ValueType::Struct:
                return data_.structure == other.data_.structure;
            default:
                return false;
        }
    }

    [[nodiscard]] bool operator!=(const Value& other) const
    {
        return !(*this == other);
    }

   private:
    ValueType type_;
    union UnionData
    {
        int64_t                      i;
        bool                         b;
        std::string_view             s;
        std::shared_ptr<ObjFunction> function;
        std::shared_ptr<ObjArray>    array;
        std::shared_ptr<ObjStruct>   structure;

        UnionData()
        {
            i = 0;
        }
        ~UnionData() {}  // handled by Value destructor
    } data_;
};

}  // namespace druk::codegen
