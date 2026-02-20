#include "druk/codegen/core/value.h"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_build_struct(const PackedValue* keys, const PackedValue* values, int32_t count,
                               PackedValue* out)
    {
        auto* s = druk::gc::GcHeap::get().alloc<druk::gc::GcStruct>();
        for (int32_t i = 0; i < count; ++i)
        {
            druk::codegen::Value k = druk::codegen::runtime::unpack_value(&keys[i]);
            if (k.isString())
                s->fields[std::string(k.asString())] =
                    druk::codegen::runtime::unpack_value(&values[i]);
        }
        druk::codegen::runtime::pack_value(druk::codegen::Value(s), out);
    }

    void druk_jit_get_field(const PackedValue* struct_val, const char* field, size_t field_len,
                            PackedValue* out)
    {
        druk::codegen::Value s = druk::codegen::runtime::unpack_value(struct_val);
        if (s.isStruct())
        {
            auto* p  = s.asGcStruct();
            auto  it = p->fields.find(std::string(field, field_len));
            if (it != p->fields.end())
            {
                druk::codegen::runtime::pack_value(it->second, out);
                return;
            }
        }
        druk_jit_value_nil(out);
    }

    void druk_jit_set_field(PackedValue* struct_val, const char* field, size_t field_len,
                            const PackedValue* val)
    {
        druk::codegen::Value s = druk::codegen::runtime::unpack_value(struct_val);
        if (s.isStruct())
            s.asGcStruct()->fields[std::string(field, field_len)] =
                druk::codegen::runtime::unpack_value(val);
    }

    void druk_jit_keys(const PackedValue* val, PackedValue* out)
    {
        druk::codegen::Value v = druk::codegen::runtime::unpack_value(val);
        if (v.isStruct())
        {
            auto* a = druk::gc::GcHeap::get().alloc<druk::gc::GcArray>();
            for (const auto& p : v.asGcStruct()->fields)
                a->elements.push_back(
                    druk::codegen::Value(druk::codegen::runtime::storeString(p.first)));
            druk::codegen::runtime::pack_value(druk::codegen::Value(a), out);
        }
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_values(const PackedValue* val, PackedValue* out)
    {
        druk::codegen::Value v = druk::codegen::runtime::unpack_value(val);
        if (v.isStruct())
        {
            auto* a = druk::gc::GcHeap::get().alloc<druk::gc::GcArray>();
            for (const auto& p : v.asGcStruct()->fields) a->elements.push_back(p.second);
            druk::codegen::runtime::pack_value(druk::codegen::Value(a), out);
        }
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_contains(const PackedValue* container, const PackedValue* item, PackedValue* out)
    {
        druk::codegen::Value c  = druk::codegen::runtime::unpack_value(container);
        druk::codegen::Value it = druk::codegen::runtime::unpack_value(item);
        if (c.isArray())
        {
            for (const auto& e : c.asGcArray()->elements)
                if (e == it)
                {
                    druk::codegen::runtime::pack_value(druk::codegen::Value(true), out);
                    return;
                }
            druk::codegen::runtime::pack_value(druk::codegen::Value(false), out);
        }
        else if (c.isStruct() && it.isString())
        {
            auto* s = c.asGcStruct();
            druk::codegen::runtime::pack_value(
                druk::codegen::Value(s->fields.find(std::string(it.asString())) != s->fields.end()),
                out);
        }
        else
            druk::codegen::runtime::pack_value(druk::codegen::Value(false), out);
    }
}
