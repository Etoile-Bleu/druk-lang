#pragma once

#include "druk/parser/ast/node.hpp"

namespace druk::parser::ast
{

struct Type : Node
{
};

struct BuiltinType : Type
{
    void accept(Visitor* v) override;
};

struct ArrayType : Type
{
    Type* elementType;
    void  accept(Visitor* v) override;
};

struct FunctionType : Type
{
    Type**   paramTypes;
    uint32_t paramCount;
    Type*    returnType;
    void     accept(Visitor* v) override;
};

}  // namespace druk::parser::ast
