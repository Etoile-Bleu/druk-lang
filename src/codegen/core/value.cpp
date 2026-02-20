#include "druk/codegen/core/value.h"

#include "druk/gc/gc_heap.h"
#include "druk/gc/types/gc_array.h"
#include "druk/gc/types/gc_string.h"
#include "druk/gc/types/gc_struct.h"

namespace druk::codegen
{

std::string_view Value::asString() const
{
    assert(type_ == ValueType::String);
    return data_.str->data;
}

bool Value::operator==(const Value& other) const
{
    if (type_ != other.type_)
        return false;
    switch (type_)
    {
        case ValueType::Nil:
            return true;
        case ValueType::Int:
            return data_.i == other.data_.i;
        case ValueType::Bool:
            return data_.b == other.data_.b;
        case ValueType::String:
            return data_.str->data == other.data_.str->data;
        case ValueType::Function:
            return data_.func == other.data_.func;
        case ValueType::Array:
            return data_.arr == other.data_.arr;
        case ValueType::Struct:
            return data_.struc == other.data_.struc;
        case ValueType::RawFunction:
            return data_.ptr == other.data_.ptr;
    }
    return false;
}

void Value::markGcRefs() const
{
    auto& heap = gc::GcHeap::get();
    switch (type_)
    {
        case ValueType::String:
            heap.markObject(data_.str);
            break;
        case ValueType::Array:
            heap.markObject(data_.arr);
            break;
        case ValueType::Struct:
            heap.markObject(data_.struc);
            break;
        case ValueType::Function:
            heap.markObject(reinterpret_cast<gc::GcObject*>(data_.func));
            break;
        default:
            break;
    }
}

}  // namespace druk::codegen
