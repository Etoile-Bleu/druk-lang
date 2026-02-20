#include "rt_internal.h"
#include "druk/codegen/core/value.h"
#include <string>
#include <sstream>
#include "druk/lexer/unicode.hpp"

using namespace druk::codegen::runtime;

extern "C"
{
    void druk_jit_to_string(const PackedValue* val, PackedValue* out)
    {
        ensureRootsRegistered();
        auto value = unpack_value(val);

        if (value.isString())
        {
            pack_value(value, out);
            return;
        }

        std::string str;
        if (value.isNil())
        {
            str = "ཅི་མེད";
        }
        else if (value.isBool())
        {
            str = value.asBool() ? "བདེན་པ་" : "རྫུན་མ་"; 
        }
        else if (value.isInt())
        {
            str = ::druk::lexer::unicode::toTibetanNumeral(value.asInt());
        }
        else if (value.isFunction() || value.isRawFunction())
        {
            str = "<function>";
        }
        else if (value.isArray())
        {
            str = "<array>";
        }
        else if (value.isStruct())
        {
            str = "<struct>";
        }
        else
        {
            str = "<unknown>";
        }

        auto* gs = storeString(str);
        pack_value(druk::codegen::Value(gs), out);
    }

    void druk_jit_string_concat(const PackedValue* l, const PackedValue* r, PackedValue* out)
    {
        ensureRootsRegistered();
        auto left  = unpack_value(l);
        auto right = unpack_value(r);

        std::string s1 = left.isString() ? std::string(left.asString()) : "";
        std::string s2 = right.isString() ? std::string(right.asString()) : "";

        auto* gs = storeString(s1 + s2);
        pack_value(druk::codegen::Value(gs), out);
    }
}
