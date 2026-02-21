#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Stmt* Parser::parseDeclaration()
{
    // 1. Function declaration: function name(
    if (check(lexer::TokenType::KwFunction) && peekNext().type == lexer::TokenType::Identifier &&
        peekNextNext().type == lexer::TokenType::LParen)
    {
        match(lexer::TokenType::KwFunction);
        return parseFunction();
    }

    // 2. Variable declaration: Type name ...
    if (check(lexer::TokenType::KwNumber) || check(lexer::TokenType::KwString) ||
        check(lexer::TokenType::KwBoolean) || check(lexer::TokenType::KwVoid) ||
        (check(lexer::TokenType::KwFunction) && peekNext().type == lexer::TokenType::Identifier))
    {
        return parseVarDeclaration();
    }

    return parseStatement();
}

ast::Stmt* Parser::parseFunction()
{
    lexer::Token name = consume(lexer::TokenType::Identifier, "Expect function name.");
    consume(lexer::TokenType::LParen, "Expect '(' after function name.");
    std::vector<ast::Param> params;
    if (!check(lexer::TokenType::RParen))
    {
        do
        {
            ast::Type*   type = parseType();
            lexer::Token paramName =
                consume(lexer::TokenType::Identifier, "Expect parameter name.");
            params.push_back({type, paramName});
        } while (match(lexer::TokenType::Comma));
    }
    consume(lexer::TokenType::RParen, "Expect ')' after parameters.");
    ast::Type* returnType = nullptr;
    if (match(lexer::TokenType::Arrow))
        returnType = parseType();
    ast::Stmt* body  = parseBlock();
    auto*      func  = arena_.make<ast::FuncDecl>();
    func->kind       = ast::NodeKind::Function;
    func->name       = name;
    func->body       = body;
    func->returnType = returnType;
    if (params.empty())
    {
        func->params     = nullptr;
        func->paramCount = 0;
    }
    else
    {
        func->paramCount = static_cast<uint32_t>(params.size());
        func->params     = arena_.allocateArray<ast::Param>(func->paramCount);
        for (size_t i = 0; i < params.size(); ++i) func->params[i] = params[i];
    }
    return func;
}

ast::Stmt* Parser::parseVarDeclaration()
{
    ast::Type*   type        = parseType();
    lexer::Token name        = consume(lexer::TokenType::Identifier, "Expect variable name.");
    ast::Expr*   initializer = nullptr;
    if (match(lexer::TokenType::Equal))
        initializer = parseExpression();
    consume(lexer::TokenType::Semicolon, "Expect ';' after variable declaration.");
    auto* var        = arena_.make<ast::VarDecl>();
    var->kind        = ast::NodeKind::Variable;
    var->name        = name;
    var->initializer = initializer;
    var->type        = type;
    return var;
}

}  // namespace druk::parser
