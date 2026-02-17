#pragma once

#include "druk/lexer/token.hpp"
#include "druk/semantic/types.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <memory>

namespace druk::semantic {

/**
 * @brief Represents a symbol (variable, function, etc.) in the symbol table.
 */
struct Symbol {
    lexer::Token token;
    Type type;
    bool isConstant = false;
    uint32_t depth = 0;
};

/**
 * @brief Represents a lexical scope containing symbols.
 */
class Scope {
public:
    explicit Scope(Scope* parent = nullptr) : parent_(parent) {}
    
    bool define(const std::string& name, const Symbol& symbol) {
        if (symbols_.find(name) != symbols_.end()) return false;
        symbols_[name] = symbol;
        return true;
    }
    
    Symbol* resolve(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) return &it->second;
        if (parent_) return parent_->resolve(name);
        return nullptr;
    }
    
    Scope* parent() const { return parent_; }

private:
    std::unordered_map<std::string, Symbol> symbols_;
    Scope* parent_;
};

/**
 * @brief Manages a stack of scopes for name resolution.
 */
class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable() = default;

    void enterScope();
    void exitScope();

    bool define(const std::string& name, const Symbol& symbol);
    Symbol* resolve(const std::string& name);
    
    uint32_t currentDepth() const { return depth_; }

private:
    std::vector<std::unique_ptr<Scope>> allScopes_; // For memory management
    Scope* currentScope_;
    uint32_t depth_;
};

} // namespace druk::semantic
