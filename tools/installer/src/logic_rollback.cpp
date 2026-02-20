#include "logic_engine.hpp"
#include <filesystem>

namespace druk::installer {

void InstallationEngine::rollback() {
    // Delete files and directories created during failed installation
    for (auto it = created_files_.rbegin(); it != created_files_.rend(); ++it) {
        std::filesystem::remove(*it);
    }
    for (auto it = created_dirs_.rbegin(); it != created_dirs_.rend(); ++it) {
        std::filesystem::remove(*it);
    }
}

}
