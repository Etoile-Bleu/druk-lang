#include "druk/parser/ast/type.hpp"

#include "druk/parser/ast/visitor.hpp"


namespace druk::parser::ast
{

void BuiltinType::accept(Visitor* v)
{
    v->visitBuiltinType(this);
}

void ArrayType::accept(Visitor* v)
{
    v->visitArrayType(this);
}

void FunctionType::accept(Visitor* v)
{
    v->visitFunctionType(this);
}

void OptionType::accept(Visitor* v)
{
    v->visitOptionType(this);
}

}  // namespace druk::parser::ast
