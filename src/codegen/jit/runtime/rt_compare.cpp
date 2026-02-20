#include "druk/codegen/core/value.h"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_equal(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::runtime::pack_value(
            druk::codegen::Value(druk::codegen::runtime::unpack_value(a) ==
                                 druk::codegen::runtime::unpack_value(b)),
            out);
    }

    void druk_jit_less(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() < vb.asInt()), out);
        else
            druk::codegen::runtime::pack_value(druk::codegen::Value(false), out);
    }

    void druk_jit_greater(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() > vb.asInt()), out);
        else
            druk::codegen::runtime::pack_value(druk::codegen::Value(false), out);
    }

    void druk_jit_less_equal(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() <= vb.asInt()), out);
        else
            druk::codegen::runtime::pack_value(druk::codegen::Value(false), out);
    }

    void druk_jit_greater_equal(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        if (va.isInt() && vb.isInt())
            druk::codegen::runtime::pack_value(druk::codegen::Value(va.asInt() >= vb.asInt()), out);
        else
            druk::codegen::runtime::pack_value(druk::codegen::Value(false), out);
    }

    void druk_jit_and(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        druk::codegen::runtime::pack_value(
            druk::codegen::Value(va.asBool() && vb.asBool()), out);
    }

    void druk_jit_or(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::Value vb = druk::codegen::runtime::unpack_value(b);
        druk::codegen::runtime::pack_value(
            druk::codegen::Value(va.asBool() || vb.asBool()), out);
    }

    void druk_jit_not(const PackedValue* a, PackedValue* out)
    {
        druk::codegen::Value va = druk::codegen::runtime::unpack_value(a);
        druk::codegen::runtime::pack_value(
            druk::codegen::Value(va.isNil() || (va.isBool() && !va.asBool())), out);
    }
}
