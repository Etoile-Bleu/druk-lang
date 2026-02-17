#include "druk/codegen/jit/jit_runtime.h"

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "druk/codegen/core/obj.h"
#include "druk/codegen/core/value.h"
#include "druk/lexer/unicode.hpp"

namespace druk::codegen
{

namespace
{
std::vector<std::string>               g_jit_args;
std::unordered_map<std::string, Value> g_globals;
std::vector<std::string>               g_string_storage;

const char* value_type_name(uint8_t type)
{
    switch (static_cast<ValueType>(type))
    {
        case ValueType::Nil:
            return "nil";
        case ValueType::Int:
            return "int";
        case ValueType::Bool:
            return "bool";
        case ValueType::String:
            return "string";
        case ValueType::Function:
            return "function";
        case ValueType::Array:
            return "array";
        case ValueType::Struct:
            return "struct";
    }
    return "unknown";
}

std::string_view store_string(std::string str)
{
    g_string_storage.push_back(std::move(str));
    return g_string_storage.back();
}

void pack_value(const Value& v, PackedValue* out)
{
    out->type = static_cast<uint8_t>(v.type());
    // Clear padding
    std::memset(out->padding, 0, sizeof(out->padding));
    out->extra = 0;
    switch (v.type())
    {
        case ValueType::Nil:
            out->data.i = 0;
            break;
        case ValueType::Int:
            out->data.i = v.asInt();
            break;
        case ValueType::Bool:
            out->data.i = v.asBool() ? 1 : 0;
            break;
        case ValueType::String:
        {
            auto sv     = v.asString();
            out->data.s = sv.data();
            out->extra  = static_cast<int64_t>(sv.size());
            break;
        }
        case ValueType::Function:
            out->data.ptr = new std::shared_ptr<ObjFunction>(v.asFunction());
            break;
        case ValueType::Array:
            out->data.ptr = new std::shared_ptr<ObjArray>(v.asArray());
            break;
        case ValueType::Struct:
            out->data.ptr = new std::shared_ptr<ObjStruct>(v.asStruct());
            break;
    }
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
            return Value(std::string_view(p->data.s, static_cast<size_t>(p->extra)));
        case ValueType::Function:
            return Value(*static_cast<std::shared_ptr<ObjFunction>*>(p->data.ptr));
        case ValueType::Array:
            return Value(*static_cast<std::shared_ptr<ObjArray>*>(p->data.ptr));
        case ValueType::Struct:
            return Value(*static_cast<std::shared_ptr<ObjStruct>*>(p->data.ptr));
    }
    return Value();
}

struct CallFrame
{
    std::vector<PackedValue> args;
};
std::vector<CallFrame>                        g_call_frames;
std::unordered_map<ObjFunction*, DrukJitFunc> g_compiled_functions;
DrukJitCompileFn                              g_compile_handler = nullptr;
}  // namespace

extern "C"
{
    void druk_jit_set_args(const char** argv, int32_t argc)
    {
        g_jit_args.clear();
        if (!argv || argc <= 0)
            return;
        for (int32_t i = 0; i < argc; ++i) g_jit_args.emplace_back(argv[i] ? argv[i] : "");
        auto argv_array = std::make_shared<ObjArray>();
        for (const auto& arg : g_jit_args) argv_array->elements.push_back(Value(store_string(arg)));
        g_globals["argv"]        = Value(argv_array);
        g_globals["argc"]        = Value(static_cast<int64_t>(g_jit_args.size()));
        g_globals["ནང་འཇུག་ཐོ"]    = Value(argv_array);
        g_globals["ནང་འཇུག་གྲངས་"] = Value(static_cast<int64_t>(g_jit_args.size()));
    }

    void druk_jit_value_nil(PackedValue* out)
    {
        out->type   = static_cast<uint8_t>(ValueType::Nil);
        out->data.i = 0;
        out->extra  = 0;
    }
    void druk_jit_value_bool(bool b, PackedValue* out)
    {
        out->type   = static_cast<uint8_t>(ValueType::Bool);
        out->data.i = b;
        out->extra  = 0;
    }
    void druk_jit_value_int(int64_t i, PackedValue* out)
    {
        out->type   = static_cast<uint8_t>(ValueType::Int);
        out->data.i = i;
        out->extra  = 0;
    }
    void druk_jit_value_function(ObjFunction* fn, PackedValue* out)
    {
        if (!fn)
        {
            druk_jit_value_nil(out);
            return;
        }
        out->type     = static_cast<uint8_t>(ValueType::Function);
        out->data.ptr = new std::shared_ptr<ObjFunction>(fn, [](ObjFunction*) {});
        out->extra    = 0;
    }

    void druk_jit_add(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt())
            pack_value(Value(va.asInt() + vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_subtract(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt())
            pack_value(Value(va.asInt() - vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_multiply(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt())
            pack_value(Value(va.asInt() * vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_divide(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt() && vb.asInt() != 0)
            pack_value(Value(va.asInt() / vb.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_negate(const PackedValue* a, PackedValue* out)
    {
        Value va = unpack_value(a);
        if (va.isInt())
            pack_value(Value(-va.asInt()), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_equal(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        pack_value(Value(unpack_value(a) == unpack_value(b)), out);
    }

    void druk_jit_less(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt())
            pack_value(Value(va.asInt() < vb.asInt()), out);
        else
            pack_value(Value(false), out);
    }

    void druk_jit_greater(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt())
            pack_value(Value(va.asInt() > vb.asInt()), out);
        else
            pack_value(Value(false), out);
    }

    void druk_jit_less_equal(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt())
            pack_value(Value(va.asInt() <= vb.asInt()), out);
        else
            pack_value(Value(false), out);
    }

    void druk_jit_greater_equal(const PackedValue* a, const PackedValue* b, PackedValue* out)
    {
        Value va = unpack_value(a), vb = unpack_value(b);
        if (va.isInt() && vb.isInt())
            pack_value(Value(va.asInt() >= vb.asInt()), out);
        else
            pack_value(Value(false), out);
    }

    void druk_jit_not(const PackedValue* a, PackedValue* out)
    {
        Value va = unpack_value(a);
        pack_value(Value(va.isNil() || (va.isBool() && !va.asBool())), out);
    }

    void druk_jit_get_global(const char* name, size_t name_len, PackedValue* out)
    {
        auto it = g_globals.find(std::string(name, name_len));
        if (it != g_globals.end())
            pack_value(it->second, out);
        else
        {
            std::cerr << "Runtime Error: Undefined variable\n";
            druk_jit_value_nil(out);
        }
    }

    void druk_jit_define_global(const char* name, size_t name_len, const PackedValue* val)
    {
        g_globals[std::string(name, name_len)] = unpack_value(val);
    }

    void druk_jit_set_global(const char* name, size_t name_len, const PackedValue* val)
    {
        auto it = g_globals.find(std::string(name, name_len));
        if (it != g_globals.end())
            it->second = unpack_value(val);
    }

    void druk_jit_build_array(const PackedValue* elements, int32_t count, PackedValue* out)
    {
        auto arr = std::make_shared<ObjArray>();
        for (int32_t i = 0; i < count; ++i) arr->elements.push_back(unpack_value(&elements[i]));
        pack_value(Value(arr), out);
    }

    void druk_jit_index(const PackedValue* arr_val, const PackedValue* idx_val, PackedValue* out)
    {
        Value arr = unpack_value(arr_val), idx = unpack_value(idx_val);
        if (arr.isArray() && idx.isInt())
        {
            auto    p = arr.asArray();
            int64_t i = idx.asInt();
            if (i >= 0 && static_cast<size_t>(i) < p->elements.size())
            {
                pack_value(p->elements[static_cast<size_t>(i)], out);
                return;
            }
        }
        druk_jit_value_nil(out);
    }

    void druk_jit_index_set(PackedValue* arr_val, const PackedValue* idx_val,
                            const PackedValue* val)
    {
        Value arr = unpack_value(arr_val), idx = unpack_value(idx_val);
        if (arr.isArray() && idx.isInt())
        {
            auto    p = arr.asArray();
            int64_t i = idx.asInt();
            if (i >= 0 && static_cast<size_t>(i) < p->elements.size())
                p->elements[static_cast<size_t>(i)] = unpack_value(val);
        }
    }

    void druk_jit_build_struct(const PackedValue* keys, const PackedValue* values, int32_t count,
                               PackedValue* out)
    {
        auto s = std::make_shared<ObjStruct>();
        for (int32_t i = 0; i < count; ++i)
        {
            Value k = unpack_value(&keys[i]);
            if (k.isString())
                s->fields[std::string(k.asString())] = unpack_value(&values[i]);
        }
        pack_value(Value(s), out);
    }

    void druk_jit_get_field(const PackedValue* struct_val, const char* field, size_t field_len,
                            PackedValue* out)
    {
        Value s = unpack_value(struct_val);
        if (s.isStruct())
        {
            auto p  = s.asStruct();
            auto it = p->fields.find(std::string(field, field_len));
            if (it != p->fields.end())
            {
                pack_value(it->second, out);
                return;
            }
        }
        druk_jit_value_nil(out);
    }

    void druk_jit_set_field(PackedValue* struct_val, const char* field, size_t field_len,
                            const PackedValue* val)
    {
        Value s = unpack_value(struct_val);
        if (s.isStruct())
            s.asStruct()->fields[std::string(field, field_len)] = unpack_value(val);
    }

    void druk_jit_len(const PackedValue* val, PackedValue* out)
    {
        Value v = unpack_value(val);
        if (v.isArray())
            pack_value(Value(static_cast<int64_t>(v.asArray()->elements.size())), out);
        else if (v.isStruct())
            pack_value(Value(static_cast<int64_t>(v.asStruct()->fields.size())), out);
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_push(PackedValue* arr_val, const PackedValue* element)
    {
        Value v = unpack_value(arr_val);
        if (v.isArray())
            v.asArray()->elements.push_back(unpack_value(element));
    }

    void druk_jit_pop_array(PackedValue* arr_val, PackedValue* out)
    {
        Value v = unpack_value(arr_val);
        if (v.isArray() && !v.asArray()->elements.empty())
        {
            pack_value(v.asArray()->elements.back(), out);
            v.asArray()->elements.pop_back();
            return;
        }
        druk_jit_value_nil(out);
    }

    void druk_jit_typeof(const PackedValue* val, PackedValue* out)
    {
        Value       v = unpack_value(val);
        const char* t = "nil";
        switch (v.type())
        {
            case ValueType::Int:
                t = "int";
                break;
            case ValueType::Bool:
                t = "bool";
                break;
            case ValueType::String:
                t = "string";
                break;
            case ValueType::Array:
                t = "array";
                break;
            case ValueType::Struct:
                t = "struct";
                break;
            case ValueType::Function:
                t = "function";
                break;
            default:
                break;
        }
        pack_value(Value(store_string(t)), out);
    }

    void druk_jit_keys(const PackedValue* val, PackedValue* out)
    {
        Value v = unpack_value(val);
        if (v.isStruct())
        {
            auto a = std::make_shared<ObjArray>();
            for (const auto& p : v.asStruct()->fields)
                a->elements.push_back(Value(store_string(p.first)));
            pack_value(Value(a), out);
        }
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_values(const PackedValue* val, PackedValue* out)
    {
        Value v = unpack_value(val);
        if (v.isStruct())
        {
            auto a = std::make_shared<ObjArray>();
            for (const auto& p : v.asStruct()->fields) a->elements.push_back(p.second);
            pack_value(Value(a), out);
        }
        else
            druk_jit_value_nil(out);
    }

    void druk_jit_contains(const PackedValue* container, const PackedValue* item, PackedValue* out)
    {
        Value c = unpack_value(container), it = unpack_value(item);
        if (c.isArray())
        {
            for (const auto& e : c.asArray()->elements)
                if (e == it)
                {
                    pack_value(Value(true), out);
                    return;
                }
            pack_value(Value(false), out);
        }
        else if (c.isStruct() && it.isString())
        {
            auto s = c.asStruct();
            pack_value(Value(s->fields.find(std::string(it.asString())) != s->fields.end()), out);
        }
        else
            pack_value(Value(false), out);
    }

    void druk_jit_input(PackedValue* out)
    {
        std::string l;
        if (std::getline(std::cin, l))
            pack_value(Value(store_string(l)), out);
        else
            pack_value(Value(store_string("")), out);
    }

    void druk_jit_print(const PackedValue* val)
    {
        Value v = unpack_value(val);
        if (v.isInt())
        {
            std::string res = ::druk::lexer::unicode::toTibetanNumeral(v.asInt());
            std::cout << res << "\n";
        }
        else if (v.isBool())
        {
            std::cout << (v.asBool() ? "བདེན" : "རྫུན") << "\n";
        }
        else if (v.isString())
        {
            std::cout << v.asString() << "\n";
        }
        else if (v.isNil())
        {
            std::cout << "nil\n";
        }
        else if (v.isArray())
        {
            std::cout << "[array:" << v.asArray()->elements.size() << "]\n";
        }
        else if (v.isStruct())
        {
            std::cout << "{struct:" << v.asStruct()->fields.size() << "}\n";
        }
        else
        {
            std::cout << "<function>\n";
        }
    }

    void druk_jit_register_function(ObjFunction* function, DrukJitFunc fn)
    {
        if (function && fn)
            g_compiled_functions[function] = fn;
    }
    void druk_jit_set_compile_handler(DrukJitCompileFn fn)
    {
        g_compile_handler = fn;
    }

    void druk_jit_call(const PackedValue* callee, const PackedValue* args, int32_t count,
                       PackedValue* out)
    {
        Value c = unpack_value(callee);
        if (!c.isFunction())
        {
            druk_jit_value_nil(out);
            return;
        }
        auto f = c.asFunction();
        if (count != f->arity)
        {
            druk_jit_value_nil(out);
            return;
        }
        auto it = g_compiled_functions.find(f.get());
        if (it == g_compiled_functions.end() && g_compile_handler)
            if (auto compiled = g_compile_handler(f.get()))
                g_compiled_functions[f.get()] = compiled;
        it = g_compiled_functions.find(f.get());
        if (it == g_compiled_functions.end())
        {
            druk_jit_value_nil(out);
            return;
        }
        CallFrame frame;
        frame.args.push_back(*callee);
        for (int32_t i = 0; i < count; ++i) frame.args.push_back(args[i]);
        g_call_frames.push_back(std::move(frame));
        it->second(out);
        g_call_frames.pop_back();
    }

    void druk_jit_get_arg(int32_t index, PackedValue* out)
    {
        if (g_call_frames.empty() || index < 0 ||
            static_cast<size_t>(index) >= g_call_frames.back().args.size())
        {
            druk_jit_value_nil(out);
            return;
        }
        *out = g_call_frames.back().args[static_cast<size_t>(index)];
    }

    int64_t druk_jit_value_as_int(const PackedValue* value)
    {
        return (value && value->type == static_cast<uint8_t>(ValueType::Int)) ? value->data.i : 0;
    }

    bool druk_jit_value_as_bool(const PackedValue* value)
    {
        if (!value)
            return false;
        if (value->type == static_cast<uint8_t>(ValueType::Bool))
            return value->data.i != 0;
        if (value->type == static_cast<uint8_t>(ValueType::Nil))
            return false;
        return true;
    }

}  // extern "C"
}  // namespace druk::codegen
