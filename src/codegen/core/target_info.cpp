#include "druk/codegen/core/target_info.h"

namespace druk::codegen
{

std::string TargetInfo::getDefaultTriple()
{
    // Should use LLVM's sys::getDefaultTargetTriple() in a real implementation
    return "x86_64-pc-windows-msvc";
}

bool TargetInfo::isWindows()
{
#ifdef _WIN32
    return true;
#else
    return false;
#endif
}

// ... other OS checks

}  // namespace druk::codegen
