#include "druk/codegen/core/value.h"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_build_array(const PackedValue* elements, int32_t count, PackedValue* out)
    {
        auto* arr = druk::gc::GcHeap::get().alloc<druk::gc::GcArray>();
        for (int32_t i = 0; i < count; ++i)
            arr->elements.push_back(druk::codegen::runtime::unpack_value(&elements[i]));
        druk::codegen::runtime::pack_value(druk::codegen::Value(arr), out);
    }

    void druk_jit_index(const PackedValue* arr_val, const PackedValue* idx_val, PackedValue* out)
    {
        druk::codegen::Value arr = druk::codegen::runtime::unpack_value(arr_val);
        druk::codegen::Value idx = druk::codegen::runtime::unpack_value(idx_val);
        if (arr.isArray() && idx.isInt())
        {
            auto*   p = arr.asGcArray();
            int64_t i = idx.asInt();
            if (i >= 0 && static_cast<size_t>(i) < p->elements.size())
            {
                druk::codegen::runtime::pack_value(p->elements[static_cast<size_t>(i)], out);
                return;
            }
        }
        druk_jit_value_nil(out);
    }

    void druk_jit_index_set(PackedValue* arr_val, const PackedValue* idx_val,
                            const PackedValue* val)
    {
        druk::codegen::Value arr = druk::codegen::runtime::unpack_value(arr_val);
        druk::codegen::Value idx = druk::codegen::runtime::unpack_value(idx_val);
        if (arr.isArray() && idx.isInt())
        {
            auto*   p = arr.asGcArray();
            int64_t i = idx.asInt();
            if (i >= 0 && static_cast<size_t>(i) < p->elements.size())
                p->elements[static_cast<size_t>(i)] = druk::codegen::runtime::unpack_value(val);
        }
    }

    void druk_jit_len(const PackedValue* val, PackedValue* out)
    {
        druk::codegen::Value v = druk::codegen::runtime::unpack_value(val);
        if (v.isArray())
            druk::codegen::runtime::pack_value(
                druk::codegen::Value(static_cast<int64_t>(v.asGcArray()->elements.size())), out);
        else if (v.isStruct())
            druk::codegen::runtime::pack_value(
                druk::codegen::Value(static_cast<int64_t>(v.asGcStruct()->fields.size())), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_push(PackedValue* arr_val, const PackedValue* element)
    {
        druk::codegen::Value v = druk::codegen::runtime::unpack_value(arr_val);
        if (v.isArray())
            v.asGcArray()->elements.push_back(druk::codegen::runtime::unpack_value(element));
    }

    void druk_jit_pop_array(PackedValue* arr_val, PackedValue* out)
    {
        druk::codegen::Value v = druk::codegen::runtime::unpack_value(arr_val);
        if (v.isArray() && !v.asGcArray()->elements.empty())
        {
            druk::codegen::runtime::pack_value(v.asGcArray()->elements.back(), out);
            v.asGcArray()->elements.pop_back();
            return;
        }
        druk_jit_value_nil(out);
    }
}
