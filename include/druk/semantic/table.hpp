#pragma once

#include "druk/semantic/symbol.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace druk {

class SymbolTable {
public:
  SymbolTable();

  void enter_scope();
  void exit_scope();

  bool define(std::string_view name, Symbol symbol);
  Symbol *resolve(std::string_view name);

private:
  std::vector<std::unordered_map<std::string_view, Symbol>> scopes_;
};

} // namespace druk
