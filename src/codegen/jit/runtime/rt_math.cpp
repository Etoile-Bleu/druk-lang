#include "druk/codegen/core/value.h"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_add(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() + vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_subtract(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() - vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_multiply(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() * vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_divide(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt() && vb.asInt() != 0)
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() / vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_negate(const PackedValue* a, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        if (va.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(-va.asInt()), out);
        else
            druk_jit_value_nil(out);
    }
}
