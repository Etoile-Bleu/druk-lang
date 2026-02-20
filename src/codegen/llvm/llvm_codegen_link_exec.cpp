#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

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
#ifdef _WIN32
    char exe_path_buf[MAX_PATH];
    GetModuleFileNameA(nullptr, exe_path_buf, MAX_PATH);
    std::filesystem::path compiler_path(exe_path_buf);
    auto parent = compiler_path.parent_path();
    std::filesystem::path lib_dir = parent;
    if (parent.filename() == "bin") {
        lib_dir = parent.parent_path() / "lib";
    } else if (std::filesystem::exists(parent / "lib")) {
        lib_dir = parent / "lib";
    }
#else
    auto lib_dir = std::filesystem::current_path() / "build" / "Release";
#endif

    std::string linker  = find_linker();
    if (linker.empty())
        return false;
    std::stringstream cmd;
    cmd << "\"";
    auto vcvars = find_vcvars(linker);
    if (!vcvars.empty())
        cmd << "call \"" << vcvars.string() << "\" >nul && ";
    cmd << "\"" << linker << "\" /nologo /OUT:\"" << exe << "\" \"" << obj << "\"";
    std::vector<std::string> libs = {"druk-core.lib",  "druk_runtime.lib", "druk_util.lib",
                                     "druk_lexer.lib", "druk_parser.lib",  "druk_semantic.lib"};
    cmd << " /LIBPATH:\"" << lib_dir.string() << "\"";
    for (const auto& lib : libs) cmd << " " << lib;
    cmd << "\"";
    return std::system(cmd.str().c_str()) == 0;
}

}  // namespace druk::codegen
