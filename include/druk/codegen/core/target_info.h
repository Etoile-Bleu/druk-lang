#pragma once

#include <string>

namespace druk::codegen
{

/**
 * @brief Provides information about the target architecture and OS.
 */
class TargetInfo
{
   public:
    static std::string getDefaultTriple();
    static bool        isWindows();
    static bool        isMacOS();
    static bool        isLinux();
};

}  // namespace druk::codegen
