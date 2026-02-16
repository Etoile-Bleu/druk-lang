#pragma once

#include "druk/common/allocator.hpp"
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace druk {

class StringInterner {
public:
  explicit StringInterner(ArenaAllocator &arena) : arena_(arena) {}

  // Returns a view to a stable, null-terminated string stored in the arena.
  // Normalized to NFC if needed (though here we assume lexer handles
  // normalization or we do strict checking).
  [[nodiscard]] std::string_view intern(std::string_view text);

private:
  struct StringHash {
    using is_transparent = void;
    size_t operator()(std::string_view sv) const {
      return std::hash<std::string_view>{}(sv);
    }
  };

  ArenaAllocator &arena_;
  // We store string_views that point to memory in the Arena.
  // The unordered_set ensures we don't duplicate.
  // Actually, std::unordered_set<std::string_view> is tricky because the view
  // must point to stable memory. We need to check if it exists, if not,
  // allocate in arena, then insert the view.
  std::unordered_set<std::string_view, StringHash> pool_;
};

namespace unicode {
// Check if a byte starts a UTF-8 sequence
[[nodiscard]] bool is_start_byte(char c);

// Get code point from UTF-8 iterator
[[nodiscard]] char32_t next_codepoint(std::string_view::const_iterator &it,
                                      std::string_view::const_iterator end);

// Validate UTF-8 string
[[nodiscard]] bool is_valid_utf8(std::string_view text);

// Convert number to Tibetan numerals
[[nodiscard]] std::string to_tibetan_numeral(int64_t n);
} // namespace unicode

} // namespace druk
