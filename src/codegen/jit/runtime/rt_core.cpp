#include "druk/codegen/core/obj.h"
#include "druk/codegen/core/value.h"
#include "druk/gc/gc_heap.h"
#include "druk/gc/types/gc_array.h"
#include "druk/gc/types/gc_string.h"
#include "druk/gc/types/gc_struct.h"
#include "rt_internal.h"


namespace druk::codegen::runtime
{

std::vector<std::string>                      g_jit_args;
std::unordered_map<std::string, Value>        g_globals;
std::vector<CallFrame>                        g_call_frames;
std::unordered_map<ObjFunction*, DrukJitFunc> g_compiled_functions;
DrukJitCompileFn                              g_compile_handler  = nullptr;
bool                                          g_roots_registered = false;

void ensureRootsRegistered()
{
    if (g_roots_registered)
        return;
    g_roots_registered = true;
    gc::GcHeap::get().roots().addSource(
        [](gc::GcObject*)
        {
            for (auto& [k, v] : g_globals) v.markGcRefs();
        });
}

gc::GcString* storeString(std::string s)
{
    return gc::GcHeap::get().alloc<gc::GcString>(std::move(s));
}

Value unpack_value(const PackedValue* p)
{
    auto type = static_cast<ValueType>(p->type);
    switch (type)
    {
        case ValueType::Nil:
            return Value();
        case ValueType::Int:
            return Value(p->data.i);
        case ValueType::Bool:
            return Value(p->data.i != 0);
        case ValueType::String:
            return Value(static_cast<gc::GcString*>(p->data.ptr));
        case ValueType::Function:
            return Value(static_cast<ObjFunction*>(p->data.ptr));
        case ValueType::Array:
            return Value(static_cast<gc::GcArray*>(p->data.ptr));
        case ValueType::Struct:
            return Value(static_cast<gc::GcStruct*>(p->data.ptr));
        case ValueType::RawFunction:
            return Value(p->data.ptr, true);
    }
    return Value();
}

void pack_value(const Value& v, PackedValue* p)
{
    p->type  = static_cast<uint8_t>(v.type());
    p->extra = 0;
    switch (v.type())
    {
        case ValueType::Nil:
            p->data.i = 0;
            break;
        case ValueType::Int:
            p->data.i = v.asInt();
            break;
        case ValueType::Bool:
            p->data.i = v.asBool() ? 1 : 0;
            break;
        case ValueType::String:
            p->data.ptr = v.asGcString();
            break;
        case ValueType::Array:
            p->data.ptr = v.asGcArray();
            break;
        case ValueType::Struct:
            p->data.ptr = v.asGcStruct();
            break;
        case ValueType::Function:
            p->data.ptr = v.asFunction();
            break;
        case ValueType::RawFunction:
            p->data.ptr = v.asRawFunction();
            break;
    }
}

}  // namespace druk::codegen::runtime
