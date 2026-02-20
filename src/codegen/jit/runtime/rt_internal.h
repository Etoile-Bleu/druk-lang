#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "druk/codegen/jit/jit_runtime.h"


#include "druk/gc/types/gc_string.h"
#include "druk/gc/types/gc_array.h"
#include "druk/gc/types/gc_struct.h"
#include "druk/gc/gc_heap.h"

extern "C" {
    void druk_jit_value_nil(PackedValue* out);
}

namespace druk::codegen
{
class Value;
struct ObjFunction;

namespace runtime
{

// Global state
extern std::vector<std::string>               g_jit_args;
extern std::unordered_map<std::string, Value> g_globals;

struct CallFrame
{
    std::vector<PackedValue> args;
};

extern std::vector<CallFrame>                        g_call_frames;
extern std::unordered_map<ObjFunction*, DrukJitFunc> g_compiled_functions;
extern DrukJitCompileFn                              g_compile_handler;

// Helpers
void          ensureRootsRegistered();
gc::GcString* storeString(std::string s);
Value         unpack_value(const PackedValue* p);
void          pack_value(const Value& v, PackedValue* p);

}  // namespace runtime
}  // namespace druk::codegen
