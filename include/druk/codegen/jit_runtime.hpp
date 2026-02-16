#pragma once

#include <cstdint>

namespace druk {
struct ObjFunction;
}

extern "C" {

// PackedValue mirrors the JIT runtime value layout.
struct PackedValue {
  uint8_t type;
  uint8_t padding[7];
  union {
    int64_t i;
    bool b;
    const char *s;
    void *ptr;
  } data;
  int64_t extra;
};

using DrukJitFunc = void (*)(PackedValue *out);
using DrukJitCompileFn = DrukJitFunc (*)(druk::ObjFunction *function);

void druk_jit_register_function(druk::ObjFunction *function, DrukJitFunc fn);
void druk_jit_set_compile_handler(DrukJitCompileFn fn);
void druk_jit_call(const PackedValue *callee,
                   const PackedValue *args,
                   int32_t arg_count,
                   PackedValue *out);
void druk_jit_get_arg(int32_t index, PackedValue *out);
int64_t druk_jit_value_as_int(const PackedValue *value);

} // extern "C"
