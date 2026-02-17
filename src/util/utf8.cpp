#include "druk/util/utf8.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

namespace druk::util::utf8
{

void initConsole()
{
#ifdef _WIN32
    // Set console output code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Enable virtual terminal processing for colors and better character handling
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode))
        {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

bool isStartByte(char c)
{
    return (static_cast<unsigned char>(c) & 0xC0) != 0x80;
}

char32_t nextCodepoint(std::string_view::const_iterator& it, std::string_view::const_iterator end)
{
    if (it == end)
        return 0;

    unsigned char c = static_cast<unsigned char>(*it++);
    if (c < 0x80)
        return c;

    char32_t res;
    int      count;

    if ((c & 0xE0) == 0xC0)
    {
        res   = c & 0x1F;
        count = 1;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        res   = c & 0x0F;
        count = 2;
    }
    else if ((c & 0xF8) == 0xF0)
    {
        res   = c & 0x07;
        count = 3;
    }
    else
        return 0xFFFD;  // Invalid

    for (int i = 0; i < count; ++i)
    {
        if (it == end)
            return 0xFFFD;
        unsigned char next = static_cast<unsigned char>(*it++);
        if ((next & 0xC0) != 0x80)
            return 0xFFFD;
        res = (res << 6) | (next & 0x3F);
    }

    return res;
}

bool isValid(std::string_view text)
{
    auto it  = text.begin();
    auto end = text.end();
    while (it != end)
    {
        if (nextCodepoint(it, end) == 0xFFFD)
            return false;
    }
    return true;
}

}  // namespace druk::util::utf8
