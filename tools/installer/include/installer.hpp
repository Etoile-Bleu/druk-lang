#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace druk::installer {

enum class WizardStep {
    Welcome,
    Maintenance, // Repair / Remove
    Options,
    Progress,
    Completion
};

enum class InstallMode {
    Install,
    Repair,
    Uninstall
};

struct InstallOptions {
    std::filesystem::path target_dir;
    bool install_compiler = true;
    bool install_llvm = true;
    bool install_stdlib = true;
    bool install_docs = true;
    bool install_examples = true;
    bool add_to_path = true;
    bool create_shortcut = true;
    bool associate_files = true;
};

class InstallerContext {
public:
    WizardStep current_step = WizardStep::Welcome;
    InstallOptions options;
    float progress = 0.0f;
    std::string current_operation;
    bool is_cancelled = false;
    bool has_error = false;
    std::string error_message;
    InstallMode mode = InstallMode::Install;

    InstallerContext() {
        target_dir_default();
        detect_mode();
    }

    void detect_mode() {
        if (std::filesystem::exists(options.target_dir / "bin" / "druk.exe")) {
            mode = InstallMode::Repair;
        }
    }

    void target_dir_default() {
        char buffer[MAX_PATH];
        if (GetEnvironmentVariableA("ProgramFiles", buffer, MAX_PATH)) {
            options.target_dir = std::filesystem::path(buffer) / "Druk";
        } else {
            options.target_dir = "C:\\Druk";
        }
    }
};

}
