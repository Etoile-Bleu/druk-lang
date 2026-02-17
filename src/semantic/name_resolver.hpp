#pragma once

#include "druk/parser/ast/stmt.hpp"
#include "druk/parser/ast/expr.hpp"
#include "druk/semantic/symbol_table.hpp"
#include "druk/util/error_handler.hpp"

namespace druk::semantic {

/**
 * @brief Visitor responsible for name resolution and basic scoping.
 */
class NameResolver {
public:
    NameResolver(util::ErrorHandler& errors, SymbolTable& table, std::string_view source);
    
    void resolve(parser::ast::Node* node);

private:
    void visit(parser::ast::Stmt* stmt);
    void visit(parser::ast::Expr* expr);

    util::ErrorHandler& errors_;
    SymbolTable& table_;
    std::string_view source_;
};

} // namespace druk::semantic
