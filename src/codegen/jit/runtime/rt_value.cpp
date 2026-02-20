#include "druk/codegen/core/value.h"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_value_nil(PackedValue* out)
    {
        out->type   = static_cast<uint8_t>(druk::codegen::ValueType::Nil);
        out->data.i = 0;
        out->extra  = 0;
    }

    void druk_jit_value_bool(bool b, PackedValue* out)
    {
        out->type   = static_cast<uint8_t>(druk::codegen::ValueType::Bool);
        out->data.i = b;
        out->extra  = 0;
    }

    void druk_jit_value_int(int64_t i, PackedValue* out)
    {
        out->type   = static_cast<uint8_t>(druk::codegen::ValueType::Int);
        out->data.i = i;
        out->extra  = 0;
    }

    void druk_jit_value_function(druk::codegen::ObjFunction* fn, PackedValue* out)
    {
        if (!fn)
        {
            druk_jit_value_nil(out);
            return;
        }
        out->type     = static_cast<uint8_t>(druk::codegen::ValueType::Function);
        out->data.ptr = fn;
        out->extra    = 0;
    }

    void druk_jit_value_raw_function(void* ptr, PackedValue* out)
    {
        if (!ptr)
        {
            druk_jit_value_nil(out);
            return;
        }
        out->type     = static_cast<uint8_t>(druk::codegen::ValueType::RawFunction);
        out->data.ptr = ptr;
        out->extra    = 0;
    }

    void druk_jit_string_literal(const char* data, size_t len, PackedValue* out)
    {
        druk::codegen::runtime::ensureRootsRegistered();
        auto* gs = druk::codegen::runtime::storeString(std::string(data, len));
        druk::codegen::runtime::pack_value(druk::codegen::Value(gs), out);
    }

    int64_t druk_jit_value_as_int(const PackedValue* value)
    {
        return (value && value->type == static_cast<uint8_t>(druk::codegen::ValueType::Int))
                   ? value->data.i
                   : 0;
    }

    int32_t druk_jit_value_as_bool_int(const PackedValue* value)
    {
        if (!value)
            return 0;

        if (value->type == static_cast<uint8_t>(druk::codegen::ValueType::Bool))
            return value->data.i != 0 ? 1 : 0;
        if (value->type == static_cast<uint8_t>(druk::codegen::ValueType::Nil))
            return 0;
        return 1;
    }
}
