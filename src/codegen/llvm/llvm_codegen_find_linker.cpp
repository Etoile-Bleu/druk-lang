#include <cstdlib>
#include <filesystem>
#include <vector>

#include "druk/codegen/llvm/llvm_codegen.h"


namespace druk::codegen
{

std::string LLVMCodeGen::find_linker()
{
#ifdef _WIN32
    if (std::system("where link.exe >nul 2>nul") == 0)
        return "link.exe";
    if (std::system("where lld-link.exe >nul 2>nul") == 0)
        return "lld-link.exe";
    const char*              pfiles   = std::getenv("ProgramFiles");
    const char*              pfiles86 = std::getenv("ProgramFiles(x86)");
    std::vector<std::string> roots;
    if (pfiles)
        roots.push_back(pfiles);
    if (pfiles86)
        roots.push_back(pfiles86);
    std::vector<std::string> years    = {"2022", "2019", "2017"};
    std::vector<std::string> editions = {"Community", "Professional", "Enterprise", "BuildTools"};
    for (const auto& root : roots)
    {
        for (const auto& year : years)
        {
            for (const auto& edition : editions)
            {
                std::filesystem::path vc = std::filesystem::path(root) / "Microsoft Visual Studio" /
                                           year / edition / "VC" / "Tools" / "MSVC";
                if (std::filesystem::exists(vc))
                {
                    for (const auto& entry : std::filesystem::directory_iterator(vc))
                    {
                        if (entry.is_directory())
                        {
                            std::filesystem::path lp =
                                entry.path() / "bin" / "Hostx64" / "x64" / "link.exe";
                            if (std::filesystem::exists(lp))
                                return lp.string();
                        }
                    }
                }
            }
        }
    }
#endif
    return "";
}

}  // namespace druk::codegen
