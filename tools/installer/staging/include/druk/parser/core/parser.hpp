#pragma once

#include <vector>

#include "druk/lexer/lexer.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/node.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/util/arena_allocator.hpp"
#include "druk/util/error_handler.hpp"

namespace druk::parser
{

class Parser
{
   public:
    Parser(std::string_view source, util::ArenaAllocator& arena, lexer::StringInterner& interner,
           util::ErrorHandler& errors);

    std::vector<ast::Stmt*> parse();

   private:
    ast::Stmt* parseDeclaration();
    ast::Stmt* parseFunction();
    ast::Stmt* parseVarDeclaration();

    ast::Stmt* parseStatement();
    ast::Stmt* parseIfStatement();
    ast::Stmt* parseMatchStatement();
    ast::Stmt* parseLoopStatement();
    ast::Stmt* parseWhileStatement();
    ast::Stmt* parseForStatement();
    ast::Stmt* parseReturnStatement();
    ast::Stmt* parsePrintStatement();
    ast::Stmt* parseExpressionStatement();
    ast::Stmt* parseBlock();

    ast::Expr* parseExpression();
    ast::Expr* parseAssignment();
    ast::Expr* parseLogicalOr();
    ast::Expr* parseLogicalAnd();
    ast::Expr* parseEquality();
    ast::Expr* parseComparison();
    ast::Expr* parseTerm();
    ast::Expr* parseFactor();
    ast::Expr* parseUnary();
    ast::Expr* parseCall(ast::Expr* callee);
    ast::Expr* parsePostfix();
    ast::Expr* parsePrimary();
    ast::Expr* parseLambda();
    ast::Expr* parseLambdaFromIdentifier(lexer::Token name);
    ast::Type* parseType();

    ast::Expr* parseArrayLiteral();
    ast::Expr* parseStructLiteral();

    bool         match(lexer::TokenType kind);
    bool         check(lexer::TokenType kind) const;
    lexer::Token advance();
    lexer::Token consume(lexer::TokenType kind, std::string_view message);
    lexer::Token peek() const;
    lexer::Token peekNext() const;
    lexer::Token peekNextNext() const;
    lexer::Token previous() const;
    bool         isAtEnd() const;
    void         synchronize();
    void         error(lexer::Token token, std::string_view message);

    lexer::Lexer           lexer_;
    util::ArenaAllocator&  arena_;
    lexer::StringInterner& interner_;
    util::ErrorHandler&    errors_;

    lexer::Token current_;
    lexer::Token next_;
    lexer::Token nextNext_;
    lexer::Token previous_;
    bool         panicMode_ = false;
};

}  // namespace druk::parser
