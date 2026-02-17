#pragma once

#include "druk/parser/ast/node.hpp"
#include "druk/parser/ast/stmt.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/semantic/symbol_table.hpp"
#include "druk/util/error_handler.hpp"
#include "druk/lexer/lexer.hpp"
#include <vector>

namespace druk::semantic {

/**
 * @brief Primary driver for semantic analysis passes.
 */
class Analyzer {
public:
    Analyzer(util::ErrorHandler& errors, lexer::StringInterner& interner, std::string_view source);
    
    /**
     * @brief Performs full semantic analysis on the provided AST.
     * @param statements The list of statements to analyze.
     * @return True if no semantic errors were found.
     */
    bool analyze(const std::vector<parser::ast::Stmt*>& statements);

private:
    util::ErrorHandler& errors_;
    lexer::StringInterner& interner_;
    std::string_view source_;
    SymbolTable table_;
};

} // namespace druk::semantic
