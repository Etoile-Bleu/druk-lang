#include "druk/parser/ast/match.hpp"
#include "druk/parser/core/parser.hpp"

namespace druk::parser
{

ast::Stmt* Parser::parseMatchStatement()
{
    ast::Expr* expression = parseExpression();
    consume(lexer::TokenType::LBrace, "Expect '{' before match arms.");

    std::vector<ast::MatchArm> arms;
    while (!check(lexer::TokenType::RBrace) && !isAtEnd())
    {
        ast::Expr* pattern = nullptr;
        if (match(lexer::TokenType::Underscore))
        {
            pattern = nullptr;
        }
        else
        {
            pattern = parseExpression();
        }
        consume(lexer::TokenType::Arrow, "Expect '->' after pattern.");
        ast::Stmt* body = parseStatement();
        arms.push_back({pattern, body});
    }

    consume(lexer::TokenType::RBrace, "Expect '}' after match arms.");

    auto* stmt       = arena_.make<ast::MatchStmt>();
    stmt->kind       = ast::NodeKind::Match;
    stmt->expression = expression;
    stmt->armCount   = static_cast<uint32_t>(arms.size());
    stmt->arms       = arena_.makeArray<ast::MatchArm>(stmt->armCount);
    for (uint32_t i = 0; i < stmt->armCount; ++i)
    {
        stmt->arms[i] = arms[i];
    }

    return stmt;
}

}  // namespace druk::parser
