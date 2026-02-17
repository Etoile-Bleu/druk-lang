#include "druk/util/interner.hpp"

#include <cstring>


namespace druk::util
{

std::string_view StringInterner::intern(std::string_view text)
{
    auto it = pool_.find(text);
    if (it != pool_.end())
    {
        return *it;
    }

    // Allocate in arena
    auto  len = text.size();
    char* mem = (char*)arena_.allocate(len + 1, alignof(char));
    std::memcpy(mem, text.data(), len);
    mem[len] = '\0';

    std::string_view view(mem, len);
    pool_.insert(view);
    return view;
}

}  // namespace druk::util
