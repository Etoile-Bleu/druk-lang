#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

#include "druk/codegen/llvm/llvm_codegen.h"


namespace druk::codegen
{

static std::filesystem::path find_vcvars(const std::string& linker)
{
    std::filesystem::path lp(linker);
    if (!lp.is_absolute())
        return "";
    auto p = lp.parent_path();
    for (int i = 0; i < 6; ++i)
        if (p.has_parent_path())
            p = p.parent_path();
    if (p.filename() == "VC")
    {
        auto candidate = p / "Auxiliary" / "Build" / "vcvars64.bat";
        if (std::filesystem::exists(candidate))
            return candidate;
    }
    return "";
}

bool LLVMCodeGen::link_executable(const std::string& obj, const std::string& exe)
{
    auto        lib_dir = std::filesystem::current_path() / "build" / "Release";
    std::string linker  = find_linker();
    if (linker.empty())
        return false;
    std::stringstream cmd;
    cmd << "\"";
    auto vcvars = find_vcvars(linker);
    if (!vcvars.empty())
        cmd << "call \"" << vcvars.string() << "\" >nul && ";
    cmd << "\"" << linker << "\" /nologo /OUT:\"" << exe << "\" \"" << obj << "\"";
    std::vector<std::string> libs = {"druk-core.lib", "druk_util.lib", "druk_lexer.lib",
                                     "druk_parser.lib", "druk_semantic.lib"};
    cmd << " /LIBPATH:\"" << lib_dir.string() << "\"";
    for (const auto& lib : libs) cmd << " " << lib;
    cmd << "\"";
    return std::system(cmd.str().c_str()) == 0;
}

}  // namespace druk::codegen
