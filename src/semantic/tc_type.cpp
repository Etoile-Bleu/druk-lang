#include "druk/parser/ast/type.hpp"
#include "type_checker.hpp"

namespace druk::semantic
{

Type TypeChecker::evaluate(parser::ast::Type* type)
{
    if (!type)
        return Type::makeError();
    visit(type);
    return currentType_;
}

void TypeChecker::visit(parser::ast::Type* type)
{
    type->accept(this);
}

void TypeChecker::visitBuiltinType(parser::ast::BuiltinType* type)
{
    auto t = type->token.type;
    if (t == lexer::TokenType::KwNumber)
        currentType_ = Type::makeInt();
    else if (t == lexer::TokenType::KwString)
        currentType_ = Type::makeString();
    else if (t == lexer::TokenType::KwBoolean)
        currentType_ = Type::makeBool();
    else if (t == lexer::TokenType::KwVoid)
        currentType_ = Type::makeVoid();
    else
        currentType_ = Type::makeError();
}

void TypeChecker::visitArrayType(parser::ast::ArrayType* type)
{
    currentType_ = Type::makeArray(evaluate(type->elementType));
}

void TypeChecker::visitFunctionType(parser::ast::FunctionType* type)
{
    std::vector<Type> params;
    for (uint32_t i = 0; i < type->paramCount; ++i)
    {
        params.push_back(evaluate(type->paramTypes[i]));
    }
    currentType_ = Type::makeFunction(std::move(params), evaluate(type->returnType));
}

}  // namespace druk::semantic
