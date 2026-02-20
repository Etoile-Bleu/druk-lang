#pragma once

#include "druk/util/interner.hpp"
#include "druk/util/utf8.hpp"

namespace druk::lexer
{

// Reuse StringInterner from util
using StringInterner = util::StringInterner;

namespace unicode
{

[[nodiscard]] bool isStartByte(char c);

[[nodiscard]] char32_t nextCodepoint(std::string_view::const_iterator& it,
                                     std::string_view::const_iterator  end);

[[nodiscard]] bool isValidUtf8(std::string_view text);

[[nodiscard]] std::string toTibetanNumeral(int64_t n);

}  // namespace unicode

}  // namespace druk::lexer
