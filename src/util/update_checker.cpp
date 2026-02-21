#include "druk/util/update_checker.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

namespace druk::util
{

static fs::path getTempPath()
{
#ifdef _WIN32
    return fs::temp_directory_path() / "druk_latest.txt";
#else
    return fs::path("/tmp/druk_latest.txt");
#endif
}

void checkUpdateAsync()
{
    auto path        = getTempPath();
    bool shouldCheck = true;

    if (fs::exists(path))
    {
        auto ftime = fs::last_write_time(path);
        auto now   = fs::file_time_type::clock::now();
        if (std::chrono::duration_cast<std::chrono::hours>(now - ftime).count() < 24)
        {
            shouldCheck = false;
        }
    }

    if (shouldCheck)
    {
#ifdef _WIN32
        std::system(
            "start /B powershell -WindowStyle Hidden -Command \"try { $r = Invoke-RestMethod -Uri "
            "'https://api.github.com/repos/Etoile-Bleu/druk-lang/releases/latest'; if "
            "($r.tag_name) { Set-Content -Path $env:TEMP\\druk_latest.txt -Value $r.tag_name } } "
            "catch {} \" >nul 2>&1");
#else
        std::system(
            "curl -s https://api.github.com/repos/Etoile-Bleu/druk-lang/releases/latest | grep "
            "'\"tag_name\":' | cut -d '\"' -f 4 > /tmp/druk_latest.txt 2>/dev/null &");
#endif
    }
}

void printUpdateNotice(const std::string& currentVersion)
{
    auto path = getTempPath();
    if (!fs::exists(path))
        return;

    std::ifstream file(path);
    std::string   latestVersion;
    if (file >> latestVersion)
    {
        if (!latestVersion.empty() && latestVersion != currentVersion)
        {
            std::cerr << "\n\033[36mnote:\033[0m A new version of Druk is available: \033[1m"
                      << latestVersion << "\033[0m (Current: " << currentVersion << ")\n"
                      << "Download it at: "
                         "https://github.com/Etoile-Bleu/druk-lang/releases/latest\n";
        }
    }
}

}  // namespace druk::util
