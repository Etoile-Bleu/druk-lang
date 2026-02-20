#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Stmt* Parser::parseStatement()
{
    if (match(lexer::TokenType::KwIf))
        return parseIfStatement();
    if (match(lexer::TokenType::KwMatch))
        return parseMatchStatement();
    if (match(lexer::TokenType::KwLoop))
        return parseLoopStatement();
    if (match(lexer::TokenType::KwWhile))
        return parseWhileStatement();
    if (match(lexer::TokenType::KwFor))
        return parseForStatement();
    if (match(lexer::TokenType::KwReturn))
        return parseReturnStatement();
    if (match(lexer::TokenType::KwPrint))
        return parsePrintStatement();
    if (check(lexer::TokenType::LBrace))
        return parseBlock();

    if (check(lexer::TokenType::KwNumber) || check(lexer::TokenType::KwString) ||
        check(lexer::TokenType::KwBoolean))
    {
        return parseVarDeclaration();
    }

    return parseExpressionStatement();
}

ast::Stmt* Parser::parseBlock()
{
    auto* block = arena_.make<ast::BlockStmt>();
    block->kind = ast::NodeKind::Block;

    consume(lexer::TokenType::LBrace, "Expect '{' before block.");

    std::vector<ast::Stmt*> stmts;
    while (!check(lexer::TokenType::RBrace) && !isAtEnd())
    {
        ast::Stmt* stmt = parseStatement();
        if (stmt)
        {
            stmts.push_back(stmt);
        }
        else
        {
            synchronize();
            if (check(lexer::TokenType::RBrace) || isAtEnd())
                break;
        }
    }
    consume(lexer::TokenType::RBrace, "Expect '}' after block.");

    if (stmts.empty())
    {
        block->statements = nullptr;
        block->count      = 0;
    }
    else
    {
        block->count      = static_cast<uint32_t>(stmts.size());
        block->statements = arena_.allocateArray<ast::Stmt*>(block->count);
        for (size_t i = 0; i < stmts.size(); ++i)
        {
            block->statements[i] = stmts[i];
        }
    }

    return block;
}

ast::Stmt* Parser::parseExpressionStatement()
{
    ast::Expr* expr = parseExpression();
    consume(lexer::TokenType::Semicolon, "Expect ';' after expression.");
    auto* stmt       = arena_.make<ast::ExpressionStmt>();
    stmt->kind       = ast::NodeKind::ExpressionStmt;
    stmt->expression = expr;
    return stmt;
}

ast::Stmt* Parser::parsePrintStatement()
{
    ast::Expr* expr = parseExpression();
    consume(lexer::TokenType::Semicolon, "Expect ';' after value.");
    auto* stmt       = arena_.make<ast::PrintStmt>();
    stmt->kind       = ast::NodeKind::Print;
    stmt->expression = expr;
    return stmt;
}

}  // namespace druk::parser
