#pragma once

#include <string_view>
#include <unordered_set>

#include "druk/util/arena_allocator.hpp"


namespace druk::util
{

/**
 * @brief StringInterner provides a way to store strings uniquely in stable memory.
 */
class StringInterner
{
   public:
    explicit StringInterner(ArenaAllocator& arena) : arena_(arena) {}

    /**
     * @brief Interns a string, returning a stable view.
     * @param text The string to intern.
     * @return A stable std::string_view pointing to arena memory.
     */
    [[nodiscard]] std::string_view intern(std::string_view text);

   private:
    struct StringHash
    {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const
        {
            return std::hash<std::string_view>{}(sv);
        }
    };

    ArenaAllocator&                                  arena_;
    std::unordered_set<std::string_view, StringHash> pool_;
};

}  // namespace druk::util
