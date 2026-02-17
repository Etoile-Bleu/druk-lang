#include "druk/lexer/unicode.hpp"

#include <algorithm>
#include <vector>

#include "druk/util/utf8.hpp"


namespace druk::lexer::unicode
{

bool isStartByte(char c)
{
    return util::utf8::isStartByte(c);
}

char32_t nextCodepoint(std::string_view::const_iterator& it, std::string_view::const_iterator end)
{
    return util::utf8::nextCodepoint(it, end);
}

bool isValidUtf8(std::string_view text)
{
    return util::utf8::isValid(text);
}

std::string toTibetanNumeral(int64_t n)
{
    if (n == 0)
        return "\xE0\xBC\xA0";  // ༠

    std::string res;
    bool        negative = n < 0;
    if (negative)
    {
        res += "-";
        n = -n;
    }

    std::vector<std::string> digits;
    while (n > 0)
    {
        int         digit = n % 10;
        std::string s;
        s.push_back((char)0xE0);
        s.push_back((char)0xBC);
        s.push_back((char)(0xA0 + digit));
        digits.push_back(s);
        n /= 10;
    }

    std::reverse(digits.begin(), digits.end());
    for (const auto& s : digits)
    {
        res += s;
    }

    return res;
}

}  // namespace druk::lexer::unicode
