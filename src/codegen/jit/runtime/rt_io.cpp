#include <iostream>
#include <string>

#include "druk/codegen/core/value.h"
#include "druk/lexer/unicode.hpp"
#include "rt_internal.h"


extern "C"
{
    void druk_jit_input(PackedValue* out)
    {
        std::string l;
        if (std::getline(std::cin, l))
            druk::codegen::runtime::pack_value(
                druk::codegen::Value(druk::codegen::runtime::storeString(std::move(l))), out);
        else
            druk::codegen::runtime::pack_value(
                druk::codegen::Value(druk::codegen::runtime::storeString("")), out);
    }

    void druk_jit_print(const PackedValue* val)
    {
        druk::codegen::Value v = druk::codegen::runtime::unpack_value(val);
        if (v.isInt())
            std::cout << ::druk::lexer::unicode::toTibetanNumeral(v.asInt()) << "\n";
        else if (v.isBool())
            std::cout << (v.asBool() ? "བདེན་པ་" : "རྫུན་མ་") << "\n";
        else if (v.isString())
            std::cout << v.asString() << "\n";
        else if (v.isNil())
            std::cout << "ཅི་མེད\n";
        else if (v.isArray())
            std::cout << "[array:" << v.asGcArray()->elements.size() << "]\n";
        else if (v.isStruct())
            std::cout << "{struct:" << v.asGcStruct()->fields.size() << "}\n";
        else
            std::cout << "<function>\n";
    }

    void druk_jit_typeof(const PackedValue* val, PackedValue* out)
    {
        druk::codegen::Value v = druk::codegen::runtime::unpack_value(val);
        const char*          t = "nil";
        switch (v.type())
        {
            case druk::codegen::ValueType::Int:
                t = "int";
                break;
            case druk::codegen::ValueType::Bool:
                t = "bool";
                break;
            case druk::codegen::ValueType::String:
                t = "string";
                break;
            case druk::codegen::ValueType::Array:
                t = "array";
                break;
            case druk::codegen::ValueType::Struct:
                t = "struct";
                break;
            case druk::codegen::ValueType::Function:
                t = "function";
                break;
            default:
                break;
        }
        druk::codegen::runtime::pack_value(
            druk::codegen::Value(druk::codegen::runtime::storeString(t)), out);
    }
}
