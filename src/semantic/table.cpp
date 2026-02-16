#include "druk/semantic/table.hpp"

namespace druk {

SymbolTable::SymbolTable() {
  enter_scope(); // Global scope
}

void SymbolTable::enter_scope() { scopes_.emplace_back(); }

void SymbolTable::exit_scope() {
  if (!scopes_.empty()) {
    scopes_.pop_back();
  }
}

bool SymbolTable::define(std::string_view name, Symbol symbol) {
  if (scopes_.empty())
    return false;
  auto &scope = scopes_.back();
  if (scope.find(name) != scope.end()) {
    return false; // Already defined in this scope
  }
  scope.insert({name, symbol});
  return true;
}

Symbol *SymbolTable::resolve(std::string_view name) {
  for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
    auto found = it->find(name);
    if (found != it->end()) {
      return &found->second;
    }
  }
  return nullptr;
}

} // namespace druk
