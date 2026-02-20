#include <iostream>

#include "druk/codegen/core/value.h"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_set_args(const char** argv, int32_t argc)
    {
        druk::codegen::runtime::ensureRootsRegistered();
        druk::codegen::runtime::g_jit_args.clear();
        if (!argv || argc <= 0)
            return;
        for (int32_t i = 0; i < argc; ++i)
            druk::codegen::runtime::g_jit_args.emplace_back(argv[i] ? argv[i] : "");

        auto* argv_array = druk::gc::GcHeap::get().alloc<druk::gc::GcArray>();
        for (const auto& arg : druk::codegen::runtime::g_jit_args)
            argv_array->elements.push_back(
                druk::codegen::Value(druk::codegen::runtime::storeString(arg)));

        druk::codegen::runtime::g_globals["argv"] = druk::codegen::Value(argv_array);
        druk::codegen::runtime::g_globals["argc"] =
            druk::codegen::Value(static_cast<int64_t>(druk::codegen::runtime::g_jit_args.size()));
    }

    void druk_jit_get_global(const char* name, size_t name_len, PackedValue* out)
    {
        druk::codegen::runtime::ensureRootsRegistered();
        auto it = druk::codegen::runtime::g_globals.find(std::string(name, name_len));
        if (it != druk::codegen::runtime::g_globals.end())
            druk::codegen::runtime::pack_value(it->second, out);
        else
        {
            std::cerr << "Runtime Error: Undefined variable\n";
            druk_jit_value_nil(out);
        }
    }

    void druk_jit_define_global(const char* name, size_t name_len, const PackedValue* val)
    {
        druk::codegen::runtime::ensureRootsRegistered();
        druk::codegen::runtime::g_globals[std::string(name, name_len)] =
            druk::codegen::runtime::unpack_value(val);
    }

    void druk_jit_set_global(const char* name, size_t name_len, const PackedValue* val)
    {
        auto it = druk::codegen::runtime::g_globals.find(std::string(name, name_len));
        if (it != druk::codegen::runtime::g_globals.end())
            it->second = druk::codegen::runtime::unpack_value(val);
    }
}
