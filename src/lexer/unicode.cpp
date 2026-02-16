#include "druk/lexer/unicode.hpp"
#include <cstring>
#ifdef EMSCRIPTEN
#include <cstdint>
#else
#include <unicode/normalizer2.h>
#include <unicode/unistr.h>
#include <unicode/utf8.h>
#include <unicode/utypes.h>
#endif

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

#ifdef EMSCRIPTEN
static char32_t decode_utf8(const uint8_t *s, size_t len, size_t &adv,
                            bool &valid) {
  adv = 0;
  valid = false;
  if (len == 0) {
    return 0;
  }
  uint8_t b0 = s[0];
  if (b0 < 0x80) {
    adv = 1;
    valid = true;
    return b0;
  }
  if ((b0 & 0xE0) == 0xC0 && len >= 2) {
    uint8_t b1 = s[1];
    if ((b1 & 0xC0) == 0x80) {
      adv = 2;
      valid = true;
      return ((b0 & 0x1F) << 6) | (b1 & 0x3F);
    }
  } else if ((b0 & 0xF0) == 0xE0 && len >= 3) {
    uint8_t b1 = s[1];
    uint8_t b2 = s[2];
    if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
      adv = 3;
      valid = true;
      return ((b0 & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F);
    }
  } else if ((b0 & 0xF8) == 0xF0 && len >= 4) {
    uint8_t b1 = s[1];
    uint8_t b2 = s[2];
    uint8_t b3 = s[3];
    if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 &&
        (b3 & 0xC0) == 0x80) {
      adv = 4;
      valid = true;
      return ((b0 & 0x07) << 18) | ((b1 & 0x3F) << 12) |
             ((b2 & 0x3F) << 6) | (b3 & 0x3F);
    }
  }
  adv = 1;
  valid = false;
  return 0xFFFD;
}
#endif

char32_t next_codepoint(std::string_view::const_iterator &it,
                        std::string_view::const_iterator end) {
  if (it == end)
    return 0;

#ifdef EMSCRIPTEN
  const uint8_t *current = reinterpret_cast<const uint8_t *>(&*it);
  size_t dist = static_cast<size_t>(end - it);
  size_t adv = 0;
  bool valid = false;
  char32_t c = decode_utf8(current, dist, adv, valid);
  it += static_cast<std::ptrdiff_t>(adv);
  if (!valid)
    return 0xFFFD;
  return c;
#else
  UChar32 c;
  const uint8_t *current = reinterpret_cast<const uint8_t *>(&*it);
  ptrdiff_t dist = end - it;
  int32_t i = 0;
  U8_NEXT(current, i, dist, c);
  it += i;
  if (c < 0)
    return 0xFFFD; // Replacement char
  return static_cast<char32_t>(c);
#endif
}

bool is_valid_utf8(std::string_view text) {
#ifdef EMSCRIPTEN
  const uint8_t *src = reinterpret_cast<const uint8_t *>(text.data());
  size_t len = text.length();
  size_t i = 0;
  while (i < len) {
    size_t adv = 0;
    bool valid = false;
    decode_utf8(src + i, len - i, adv, valid);
    if (!valid)
      return false;
    i += adv;
  }
  return true;
#else
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
#endif
}

std::string to_tibetan_numeral(int64_t n) {
  if (n == 0)
    return "\xE0\xBC\xA0"; // ༠

  std::string result;
  bool negative = false;
  if (n < 0) {
    negative = true;
    n = -n;
  }

  while (n > 0) {
    int digit = n % 10;
    // Tibetan digits start at U+0F20
    std::string tib_digit = "\xE0\xBC";
    tib_digit += static_cast<char>(0xA0 + digit);
    result = tib_digit + result;
    n /= 10;
  }

  if (negative) {
    result =
        "-" + result; // Or Tibetan negative sign? ASCII '-' is fine for now.
  }

  return result;
}
} // namespace unicode

} // namespace druk
