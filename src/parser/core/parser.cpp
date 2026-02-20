#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

Parser::Parser(std::string_view source, util::ArenaAllocator& arena,
               lexer::StringInterner& interner, util::ErrorHandler& errors)
    : lexer_(source, arena, interner, errors),
      arena_(arena),
      interner_(interner),
      errors_(errors),
      panicMode_(false)
{
    current_  = lexer_.next();
    next_     = lexer_.next();
    nextNext_ = lexer_.next();
}

std::vector<ast::Stmt*> Parser::parse()
{
    std::vector<ast::Stmt*> statements;
    while (!isAtEnd())
    {
        ast::Stmt* stmt = parseDeclaration();
        if (stmt)
            statements.push_back(stmt);
        else
            synchronize();
    }
    return statements;
}

lexer::Token Parser::advance()
{
    previous_ = current_;
    current_  = next_;
    next_     = nextNext_;
    if (next_.type != lexer::TokenType::EndOfFile)
    {
        nextNext_ = lexer_.next();
    }
    return previous_;
}

bool Parser::check(lexer::TokenType kind) const
{
    if (isAtEnd())
        return false;
    return current_.type == kind;
}

lexer::Token Parser::consume(lexer::TokenType kind, std::string_view message)
{
    if (check(kind))
        return advance();
    error(current_, message);
    if (current_.type != lexer::TokenType::EndOfFile)
        advance();
    return current_;
}

bool Parser::match(lexer::TokenType kind)
{
    if (check(kind))
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::isAtEnd() const
{
    return current_.type == lexer::TokenType::EndOfFile;
}
lexer::Token Parser::peek() const
{
    return current_;
}
lexer::Token Parser::peekNext() const
{
    return next_;
}
lexer::Token Parser::peekNextNext() const
{
    return nextNext_;
}
lexer::Token Parser::previous() const
{
    return previous_;
}

}  // namespace druk::parser
