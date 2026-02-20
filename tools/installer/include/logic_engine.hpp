#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include "installer.hpp"

namespace druk::installer {

class InstallationEngine {
public:
    InstallationEngine(InstallerContext& context);
    
    bool run();
    bool uninstall();
    void rollback();

private:
    bool create_directories();
    bool extract_files();
    bool update_system_path();
    bool remove_system_path();
    bool register_uninstaller();
    bool unregister_uninstaller();
    bool create_shortcuts();
    bool associate_file_extensions();
    bool remove_file_associations();

    InstallerContext& context_;
    std::vector<std::filesystem::path> created_dirs_;
    std::vector<std::filesystem::path> created_files_;
};

}
