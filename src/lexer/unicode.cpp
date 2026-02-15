#include "druk/lexer/unicode.hpp"
#include <cstring>
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>
#include <unicode/utf8.h>

namespace druk {

std::string_view StringInterner::intern(std::string_view text) {
  auto it = pool_.find(text);
  if (it != pool_.end()) {
    return *it;
  }

  // Allocate in arena
  // We allocate text.size() + 1 for null terminator, just in case C-compat is
  // needed
  auto len = text.size();
  char *mem = (char *)arena_.allocate(len + 1, alignof(char));
  std::memcpy(mem, text.data(), len);
  mem[len] = '\0';

  std::string_view view(mem, len);
  pool_.insert(view);
  return view;
}

namespace unicode {
bool is_start_byte(char c) { return (c & 0xC0) != 0x80; }

char32_t next_codepoint(std::string_view::const_iterator &it,
                        std::string_view::const_iterator end) {
  if (it == end)
    return 0;

  UChar32 c;

  // We need to convert iterator to offset for ICU macros, or just handle
  // manually. ICU Macros U8_NEXT expect a raw array. Let's use the ICU macros
  // safely. But U8_NEXT advances an index.

  // Simpler manual implementation or use ICU U8_NEXT_UNSAFE if we validated
  // before. Let's use strict manual extraction for safety or standard
  // implementation. C++20 doesn't have robust unicode updates yet (std::text is
  // C++23/26?).

  const uint8_t *current = reinterpret_cast<const uint8_t *>(&*it);
  // Distance to end
  ptrdiff_t dist = end - it;

  int32_t i = 0;
  U8_NEXT(current, i, dist, c);

  it += i;
  if (c < 0)
    return 0xFFFD; // Replacement char
  return static_cast<char32_t>(c);
}

bool is_valid_utf8(std::string_view text) {
  const uint8_t *src = reinterpret_cast<const uint8_t *>(text.data());
  int32_t len = static_cast<int32_t>(text.length());
  int32_t i = 0;
  UChar32 c;
  while (i < len) {
    U8_NEXT(src, i, len, c);
    if (c < 0)
      return false;
  }
  return true;
}
} // namespace unicode

} // namespace druk
