#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace druk::semantic
{

/**
 * @brief Categorizes the fundamental types in the language.
 */
enum class TypeKind
{
    Void,
    Int,
    String,
    Bool,
    Function,
    Array,
    Struct,
    Option,
    Error
};

struct Type;

/**
 * @brief Represents a field in a struct type.
 */
struct StructField
{
    std::string           name;
    std::shared_ptr<Type> type;
};

/**
 * @brief Represents a type in the Druk language.
 */
struct Type
{
    TypeKind kind;

    std::shared_ptr<Type>    elementType;
    std::shared_ptr<Type>    returnType;
    std::vector<Type>        paramTypes;
    std::vector<StructField> fields;

    bool operator==(const Type& other) const;
    bool operator!=(const Type& other) const
    {
        return !(*this == other);
    }

    static Type makeInt()
    {
        return {TypeKind::Int};
    }
    static Type makeString()
    {
        return {TypeKind::String};
    }
    static Type makeBool()
    {
        return {TypeKind::Bool};
    }
    static Type makeVoid()
    {
        return {TypeKind::Void};
    }
    static Type makeError()
    {
        return {TypeKind::Error};
    }

    static Type makeArray(Type element)
    {
        Type t{TypeKind::Array};
        t.elementType = std::make_shared<Type>(element);
        return t;
    }

    static Type makeOption(Type element)
    {
        Type t{TypeKind::Option};
        t.elementType = std::make_shared<Type>(element);
        return t;
    }

    static Type makeStruct(std::vector<StructField> fields)
    {
        Type t{TypeKind::Struct};
        t.fields = std::move(fields);
        return t;
    }

    static Type makeFunction(std::vector<Type> params, Type ret)
    {
        Type t{TypeKind::Function};
        t.paramTypes = std::move(params);
        t.returnType = std::make_shared<Type>(ret);
        return t;
    }
};

/**
 * @brief Converts a type to its string representation.
 */
std::string typeToString(const Type& type);

}  // namespace druk::semantic
