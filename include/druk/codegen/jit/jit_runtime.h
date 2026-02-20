#pragma once

#include <cstdint>

namespace druk::codegen
{
struct ObjFunction;
}

extern "C"
{
    struct PackedValue
    {
        uint8_t type;
        uint8_t padding[7];
        union
        {
            int64_t     i;
            bool        b;
            const char* s;
            void*       ptr;
        } data;
        int64_t extra;
    };

    using DrukJitFunc      = void (*)(PackedValue* out);
    using DrukJitCompileFn = DrukJitFunc (*)(druk::codegen::ObjFunction* function);

    void    druk_jit_set_args(const char** argv, int32_t argc);
    void    druk_jit_register_function(druk::codegen::ObjFunction* function, DrukJitFunc fn);
    void    druk_jit_set_compile_handler(DrukJitCompileFn fn);
    void    druk_jit_call(const PackedValue* callee, const PackedValue* args, int32_t arg_count,
                          PackedValue* out);
    void    druk_jit_get_arg(int32_t index, PackedValue* out);
    int64_t druk_jit_value_as_int(const PackedValue* value);
    int32_t druk_jit_value_as_bool_int(const PackedValue* value);
    void    druk_jit_string_literal(const char* data, size_t len, PackedValue* out);
    void    druk_jit_value_raw_function(void* ptr, PackedValue* out);

}  // extern "C"
