#include "druk/codegen/core/obj.h"
#include "druk/codegen/core/value.h"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_register_function(druk::codegen::ObjFunction* function, DrukJitFunc fn)
    {
        if (function && fn)
            druk::codegen::runtime::g_compiled_functions[function] = fn;
    }

    void druk_jit_set_compile_handler(DrukJitCompileFn fn)
    {
        druk::codegen::runtime::g_compile_handler = fn;
    }

    void druk_jit_call(const PackedValue* callee, const PackedValue* args, int32_t count,
                       PackedValue* out)
    {
        druk::codegen::Value c = druk::codegen::runtime::unpack_value(callee);
        if (c.isRawFunction())
        {
            druk::codegen::runtime::CallFrame frame;
            frame.args.push_back(*callee);
            for (int32_t i = 0; i < count; ++i) frame.args.push_back(args[i]);
            druk::codegen::runtime::g_call_frames.push_back(std::move(frame));
            reinterpret_cast<DrukJitFunc>(c.asRawFunction())(out);
            druk::codegen::runtime::g_call_frames.pop_back();
            return;
        }
        if (!c.isFunction())
        {
            druk_jit_value_nil(out);
            return;
        }
        auto* f = c.asFunction();
        if (count != f->arity)
        {
            druk_jit_value_nil(out);
            return;
        }
        auto it = druk::codegen::runtime::g_compiled_functions.find(f);
        if (it == druk::codegen::runtime::g_compiled_functions.end() &&
            druk::codegen::runtime::g_compile_handler)
            if (auto compiled = druk::codegen::runtime::g_compile_handler(f))
                druk::codegen::runtime::g_compiled_functions[f] = compiled;

        it = druk::codegen::runtime::g_compiled_functions.find(f);
        if (it == druk::codegen::runtime::g_compiled_functions.end())
        {
            druk_jit_value_nil(out);
            return;
        }
        druk::codegen::runtime::CallFrame frame;
        frame.args.push_back(*callee);
        for (int32_t i = 0; i < count; ++i) frame.args.push_back(args[i]);
        druk::codegen::runtime::g_call_frames.push_back(std::move(frame));
        it->second(out);
        druk::codegen::runtime::g_call_frames.pop_back();
    }

    void druk_jit_get_arg(int32_t index, PackedValue* out)
    {
        if (druk::codegen::runtime::g_call_frames.empty() || index < 0 ||
            static_cast<size_t>(index) >= druk::codegen::runtime::g_call_frames.back().args.size())
        {
            druk_jit_value_nil(out);
            return;
        }
        *out = druk::codegen::runtime::g_call_frames.back().args[static_cast<size_t>(index)];
    }
}
