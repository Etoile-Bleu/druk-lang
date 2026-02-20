#include "druk/semantic/types.hpp"

namespace druk::semantic {

bool Type::operator==(const Type& other) const {
    if (kind != other.kind) return false;
    
    if (kind == TypeKind::Array || kind == TypeKind::Option) {
        if (!elementType || !other.elementType) return elementType == other.elementType;
        return *elementType == *other.elementType;
    }
    
    if (kind == TypeKind::Struct) {
        if (fields.size() != other.fields.size()) return false;
        for (size_t i = 0; i < fields.size(); ++i) {
            if (fields[i].name != other.fields[i].name) return false;
            if (!fields[i].type || !other.fields[i].type) {
                if (fields[i].type != other.fields[i].type) return false;
                continue;
            }
            if (*fields[i].type != *other.fields[i].type) return false;
        }
    }
    
    return true;
}

std::string typeToString(const Type& type) {
    switch (type.kind) {
        case TypeKind::Void: return "void";
        case TypeKind::Int: return "number";
        case TypeKind::String: return "string";
        case TypeKind::Bool: return "boolean";
        case TypeKind::Function: return "function";
        case TypeKind::Array:
            return "Array<" + (type.elementType ? typeToString(*type.elementType) : "unknown") + ">";
        case TypeKind::Option:
            return (type.elementType ? typeToString(*type.elementType) : "unknown") + "?";
        case TypeKind::Struct:
            return "struct";
        case TypeKind::Error: return "error";
    }
    return "unknown";
}

} // namespace druk::semantic
