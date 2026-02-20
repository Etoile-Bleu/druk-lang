#include "druk/semantic/symbol_table.hpp"

namespace druk::semantic {

SymbolTable::SymbolTable() : depth_(0) {
    // Create global scope
    auto global = std::make_unique<Scope>();
    currentScope_ = global.get();
    allScopes_.push_back(std::move(global));
}

void SymbolTable::enterScope() {
    auto scope = std::make_unique<Scope>(currentScope_);
    currentScope_ = scope.get();
    allScopes_.push_back(std::move(scope));
    depth_++;
}

void SymbolTable::exitScope() {
    if (currentScope_->parent()) {
        currentScope_ = currentScope_->parent();
        depth_--;
    }
}

bool SymbolTable::define(const std::string& name, const Symbol& symbol) {
    return currentScope_->define(name, symbol);
}

Symbol* SymbolTable::resolve(const std::string& name) {
    return currentScope_->resolve(name);
}

Symbol* SymbolTable::resolveLocal(const std::string& name) {
    return currentScope_->resolveLocal(name);
}

} // namespace druk::semantic
