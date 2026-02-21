#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <string_view>

namespace druk
{
namespace gc
{
class GcArray;
class GcString;
class GcStruct;
class GcHeap;
}  // namespace gc
}  // namespace druk

namespace druk
{
namespace codegen
{

struct ObjFunction;

enum class ValueType : uint8_t
{
    Nil,
    Int,
    Bool,
    String,
    Function,
    Array,
    Struct,
    RawFunction,
};

class Value
{
   public:
    Value() : type_(ValueType::Nil)
    {
        data_.i = 0;
    }
    explicit Value(int64_t v) : type_(ValueType::Int)
    {
        data_.i = v;
    }
    explicit Value(bool v) : type_(ValueType::Bool)
    {
        data_.b = v;
    }
    explicit Value(gc::GcString* v);
    explicit Value(gc::GcArray* v);
    explicit Value(gc::GcStruct* v);
    explicit Value(ObjFunction* v);
    explicit Value(void* v, bool isRaw) : type_(ValueType::RawFunction)
    {
        data_.ptr = v;
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
    [[nodiscard]] bool isRawFunction() const
    {
        return type_ == ValueType::RawFunction;
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

    [[nodiscard]] gc::GcString* asGcString() const
    {
        assert(type_ == ValueType::String);
        return data_.str;
    }

    [[nodiscard]] gc::GcArray* asGcArray() const
    {
        assert(type_ == ValueType::Array);
        return data_.arr;
    }

    [[nodiscard]] gc::GcStruct* asGcStruct() const
    {
        assert(type_ == ValueType::Struct);
        return data_.struc;
    }

    [[nodiscard]] ObjFunction* asFunction() const
    {
        assert(type_ == ValueType::Function);
        return data_.func;
    }

    [[nodiscard]] void* asRawFunction() const
    {
        assert(type_ == ValueType::RawFunction);
        return data_.ptr;
    }

    [[nodiscard]] std::string_view asString() const;

    [[nodiscard]] bool operator==(const Value& other) const;
    [[nodiscard]] bool operator!=(const Value& other) const
    {
        return !(*this == other);
    }

    void markGcRefs() const;

   private:
    ValueType type_;
    union
    {
        int64_t       i;
        bool          b;
        gc::GcString* str;
        gc::GcArray*  arr;
        gc::GcStruct* struc;
        ObjFunction*  func;
        void*         ptr;
    } data_;
};

}  // namespace codegen
}  // namespace druk
