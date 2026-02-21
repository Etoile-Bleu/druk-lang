#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>

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
    std::filesystem::path bin_dir;
#ifdef _WIN32
    char exe_path_buf[MAX_PATH];
    if (GetModuleFileNameA(nullptr, exe_path_buf, MAX_PATH) == 0)
        bin_dir = std::filesystem::current_path();
    else
        bin_dir = std::filesystem::path(exe_path_buf).parent_path();
#else
    char    result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1)
        bin_dir =
            std::filesystem::path(std::string(result, static_cast<size_t>(count))).parent_path();
    else
        bin_dir = std::filesystem::current_path();
#endif

    std::filesystem::path lib_dir = bin_dir;

    // Robust check for library location
#ifdef _WIN32
    std::string test_lib = "druk-core.lib";
#else
    std::string test_lib = "libdruk-core.a";
#endif

    if (!std::filesystem::exists(bin_dir / test_lib))
    {
        if (bin_dir.filename() == "bin")
        {
            if (std::filesystem::exists(bin_dir.parent_path() / "lib" / test_lib))
                lib_dir = bin_dir.parent_path() / "lib";
        }
        else if (std::filesystem::exists(bin_dir / "lib" / test_lib))
        {
            lib_dir = bin_dir / "lib";
        }
    }

    std::string linker = find_linker();
    if (linker.empty())
    {
        std::cerr << "Linker not found.\n";
        return false;
    }

    std::stringstream cmd;
#ifdef _WIN32
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
#else
    cmd << linker << " -o \"" << exe << "\" \"" << obj << "\"";
    // On Linux, libraries are usually libdruk-core.a, etc.
    // And we need -L for lib path and -l for each lib.
    cmd << " -L\"" << lib_dir.string() << "\"";
    // Note: link order matters!
    std::vector<std::string> libs = {"druk-core",  "druk_parser",  "druk_semantic",
                                     "druk_lexer", "druk_runtime", "druk_util"};
    for (const auto& lib : libs) cmd << " -l" << lib;

    // Add required system libs for AOT
    cmd << " -lstdc++ -lm -luuid -lz -lzstd -licuuc -licudata";

    // LLVM libs might be needed too if not statically linked into druk-core
    // But docker build usually links them in. Let's see.
#endif

    if (debug_)
        std::cout << "Linking: " << cmd.str() << "\n";

    return std::system(cmd.str().c_str()) == 0;
}

}  // namespace druk::codegen
