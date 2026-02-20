#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace druk::util
{

/**
 * @brief Simple UTF-8 utilities for Dzongkha support.
 */
namespace utf8
{

// Initialize console for UTF-8 on Windows
void initConsole();

// Check if a byte starts a UTF-8 sequence
bool isStartByte(char c);

// Decode next codepoint from UTF-8 string
char32_t nextCodepoint(std::string_view::const_iterator& it, std::string_view::const_iterator end);

// Validate UTF-8
bool isValid(std::string_view text);

}  // namespace utf8

}  // namespace druk::util
