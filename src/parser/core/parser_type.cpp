#include "druk/parser/ast/type.hpp"
#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Type* Parser::parseType()
{
    ast::Type* type = nullptr;

    if (match(lexer::TokenType::LParen))
    {
        std::vector<ast::Type*> paramTypes;
        if (!check(lexer::TokenType::RParen))
        {
            do
            {
                paramTypes.push_back(parseType());
            } while (match(lexer::TokenType::Comma));
        }
        consume(lexer::TokenType::RParen, "Expect ')' after parameter types.");
        consume(lexer::TokenType::Arrow, "Expect '->' after parameter types.");
        ast::Type* returnType = parseType();

        auto* funcType       = arena_.make<ast::FunctionType>();
        funcType->kind       = ast::NodeKind::FunctionType;
        funcType->paramCount = static_cast<uint32_t>(paramTypes.size());
        funcType->paramTypes = arena_.allocateArray<ast::Type*>(funcType->paramCount);
        for (uint32_t i = 0; i < funcType->paramCount; ++i)
        {
            funcType->paramTypes[i] = paramTypes[i];
        }
        funcType->returnType = returnType;
        type                 = funcType;
    }
    else if (match(lexer::TokenType::KwNumber) || match(lexer::TokenType::KwString) ||
             match(lexer::TokenType::KwBoolean) || match(lexer::TokenType::KwVoid))
    {
        auto* builtin  = arena_.make<ast::BuiltinType>();
        builtin->kind  = ast::NodeKind::BuiltinType;
        builtin->token = previous();
        type           = builtin;
    }
    else
    {
        error(peek(), "Expect type name.");
        return nullptr;
    }

    while (match(lexer::TokenType::LBracket))
    {
        consume(lexer::TokenType::RBracket, "Expect ']' after '[' for array type.");
        auto* arrayType        = arena_.make<ast::ArrayType>();
        arrayType->kind        = ast::NodeKind::ArrayType;
        arrayType->elementType = type;
        type                   = arrayType;
    }

    return type;
}

}  // namespace druk::parser
