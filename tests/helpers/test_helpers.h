#pragma once

// ─── Shared test utilities for Druk lang unit tests ───────────────────────────
// Include this header in any test file. It provides:
//   - ParseHelper   : lex + parse in one step
//   - SemanticHelper: parse + analyze in one step
//   - Common type aliases

#include <gtest/gtest.h>

#include <string_view>
#include <vector>

#include "druk/lexer/lexer.hpp"
#include "druk/lexer/token.hpp"
#include "druk/lexer/unicode.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/parser/core/parser.hpp"
#include "druk/semantic/analyzer.hpp"
#include "druk/util/arena_allocator.hpp"
#include "druk/util/error_handler.hpp"


namespace druk::test
{

// ─── Type aliases ─────────────────────────────────────────────────────────────
using TT  = druk::lexer::TokenType;
using Tok = druk::lexer::Token;

// ─── LexHelper ────────────────────────────────────────────────────────────────
// Tokenises a source string and returns all non-EOF tokens.
struct LexHelper
{
    druk::util::ArenaAllocator  arena;
    druk::lexer::StringInterner interner{arena};
    druk::util::ErrorHandler    errors;

    std::vector<druk::lexer::Token> tokenize(std::string_view src)
    {
        druk::lexer::Lexer              lexer(src, arena, interner, errors);
        std::vector<druk::lexer::Token> tokens;
        while (true)
        {
            auto tok = lexer.next();
            if (tok.type == TT::EndOfFile)
                break;
            tokens.push_back(tok);
        }
        return tokens;
    }

    // Returns the first token scanned from src.
    druk::lexer::Token first(std::string_view src)
    {
        druk::lexer::Lexer lexer(src, arena, interner, errors);
        return lexer.next();
    }

    bool hasErrors() const
    {
        return errors.hasErrors();
    }
};

// ─── ParseHelper ──────────────────────────────────────────────────────────────
// Lexes + parses a source string. Access stmts and errors directly.
struct ParseHelper
{
    druk::util::ArenaAllocator  arena;
    druk::lexer::StringInterner interner{arena};
    druk::util::ErrorHandler    errors;

    std::vector<druk::parser::ast::Stmt*> parse(std::string_view src)
    {
        druk::parser::Parser p(src, arena, interner, errors);
        return p.parse();
    }

    bool hasErrors() const
    {
        return errors.hasErrors();
    }
    bool noErrors() const
    {
        return !errors.hasErrors();
    }
};

// ─── SemanticHelper ───────────────────────────────────────────────────────────
// Lex + parse + semantic analysis in one call.
struct SemanticHelper
{
    druk::util::ArenaAllocator  arena;
    druk::lexer::StringInterner interner{arena};
    druk::util::ErrorHandler    errors;

    // Returns true if semantic analysis passes.
    bool analyze(std::string_view src)
    {
        druk::parser::Parser p(src, arena, interner, errors);
        auto                 stmts = p.parse();
        if (errors.hasErrors())
            return false;
        druk::semantic::Analyzer analyzer(errors, interner, src);
        return analyzer.analyze(stmts);
    }

    bool hasErrors() const
    {
        return errors.hasErrors();
    }
};

}  // namespace druk::test
