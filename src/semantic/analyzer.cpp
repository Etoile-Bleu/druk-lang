#include "druk/semantic/analyzer.hpp"
#include "name_resolver.hpp"
#include "type_checker.hpp"

namespace druk::semantic {

Analyzer::Analyzer(util::ErrorHandler& errors, lexer::StringInterner& interner, std::string_view source)
    : errors_(errors), interner_(interner), source_(source) {}

bool Analyzer::analyze(const std::vector<parser::ast::Stmt*>& statements) {
    // Phase 1: Name Resolution (Scoping and binding)
    NameResolver resolver(errors_, table_, source_);
    for (auto* stmt : statements) {
        resolver.resolve(stmt);
    }
    
    if (errors_.hasErrors()) return false;
    
    // Phase 2: Type Checking
    TypeChecker checker(errors_, table_, source_);
    for (auto* stmt : statements) {
        checker.check(stmt);
    }
    
    return !errors_.hasErrors();
}

} // namespace druk::semantic
