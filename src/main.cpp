#include "druk/common/allocator.hpp"
#include "druk/lexer/token.hpp"
#include "druk/lexer/unicode.hpp"
#include <iostream>


int main() {
  druk::ArenaAllocator arena;
  druk::StringInterner interner(arena);

  std::cout << "Druk Language Compiler - Foundation Phase\n";
  std::cout << "Arena initialized.\n";

  std::string_view hello = interner.intern("Hello Druk!");
  std::string_view hello2 = interner.intern("Hello Druk!");

  if (hello.data() == hello2.data()) {
    std::cout << "String interning works!\n";
  } else {
    std::cerr << "String interning failed!\n";
    return 1;
  }

  std::cout << "Token size: " << sizeof(druk::Token) << " bytes\n";

  return 0;
}
