#include "logic_engine.hpp"
#include "util_logger.hpp"
#include "../resource.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <fstream>

namespace druk::installer {

InstallationEngine::InstallationEngine(InstallerContext& context) : context_(context) {
    Logger::instance().info("InstallationEngine initialized.");
}

bool InstallationEngine::run() {
    Logger::instance().info("Starting installation process...");
    context_.progress = 0.0f;
    
    context_.current_operation = "Creating directories...";
    Logger::instance().info(context_.current_operation);
    if (!create_directories()) {
        Logger::instance().error("Failed to create directories.");
        return false;
    }
    context_.progress = 0.2f;

    context_.current_operation = "Extracting files...";
    Logger::instance().info(context_.current_operation);
    if (!extract_files()) {
        Logger::instance().error("Failed to extract files.");
        return false;
    }
    context_.progress = 0.6f;

    context_.current_operation = "Updating system PATH...";
    Logger::instance().info(context_.current_operation);
    if (context_.options.add_to_path) {
        if (!update_system_path()) Logger::instance().warn("Failed to update system PATH.");
    }
    context_.progress = 0.8f;

    context_.current_operation = "Creating shortcuts...";
    Logger::instance().info(context_.current_operation);
    if (context_.options.create_shortcut) {
        if (!create_shortcuts()) Logger::instance().warn("Failed to create shortcuts.");
    }
    context_.progress = 0.9f;

    context_.current_operation = "Finalizing registration...";
    Logger::instance().info(context_.current_operation);
    register_uninstaller();
    if (context_.options.associate_files) associate_file_extensions();
    
    context_.progress = 1.0f;
    context_.current_operation = "Installation complete!";
    Logger::instance().info(context_.current_operation);
    return true;
}

bool InstallationEngine::uninstall() {
    Logger::instance().info("Starting uninstallation process...");
    context_.progress = 0.0f;

    context_.current_operation = "Removing file associations...";
    Logger::instance().info(context_.current_operation);
    remove_file_associations();
    context_.progress = 0.2f;

    context_.current_operation = "Removing system PATH...";
    Logger::instance().info(context_.current_operation);
    remove_system_path();
    context_.progress = 0.4f;

    context_.current_operation = "Unregistering uninstaller...";
    Logger::instance().info(context_.current_operation);
    unregister_uninstaller();
    context_.progress = 0.6f;

    context_.current_operation = "Deleting files...";
    Logger::instance().info(context_.current_operation);
    try {
        if (std::filesystem::exists(context_.options.target_dir)) {
            std::filesystem::remove_all(context_.options.target_dir);
        }
    } catch (const std::exception& e) {
        Logger::instance().error(std::format("Failed to delete files: {}", e.what()));
    }
    context_.progress = 1.0f;

    context_.current_operation = "Uninstallation complete!";
    Logger::instance().info(context_.current_operation);
    return true;
}

bool InstallationEngine::create_directories() {
    try {
        std::filesystem::create_directories(context_.options.target_dir);
        std::filesystem::create_directories(context_.options.target_dir / "bin");
        std::filesystem::create_directories(context_.options.target_dir / "lib");
        std::filesystem::create_directories(context_.options.target_dir / "include");
        std::filesystem::create_directories(context_.options.target_dir / "docs");
        std::filesystem::create_directories(context_.options.target_dir / "examples");
        return true;
    } catch (...) {
        return false;
    }
}

bool InstallationEngine::extract_files() {
    try {
        Logger::instance().info("Loading embedded payload resource...");
        HMODULE hModule = GetModuleHandle(nullptr);
        HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(IDR_PAYLOAD_ZIP), RT_RCDATA);
        if (!hResource) {
            Logger::instance().error("Failed to find payload resource.");
            return false;
        }

        HGLOBAL hMemory = LoadResource(hModule, hResource);
        if (!hMemory) {
            Logger::instance().error("Failed to load payload resource.");
            return false;
        }

        DWORD size = SizeofResource(hModule, hResource);
        void* ptr = LockResource(hMemory);

        if (!ptr || size == 0) {
            Logger::instance().error("Failed to lock payload resource or size is 0.");
            return false;
        }

        std::filesystem::path zip_path = context_.options.target_dir / "payload.zip";
        Logger::instance().info(std::format("Writing payload to {}", zip_path.string()));
        
        std::ofstream outfile(zip_path, std::ios::binary);
        if (!outfile.is_open()) {
            Logger::instance().error("Failed to open payload.zip for writing.");
            return false;
        }
        outfile.write(reinterpret_cast<const char*>(ptr), size);
        outfile.close();

        Logger::instance().info("Extracting payload...");
        
        // Extract the zip using PowerShell
        std::string cmd = std::format("powershell -NoProfile -NonInteractive -Command \"Expand-Archive -Path '{}' -DestinationPath '{}' -Force\"", zip_path.string(), context_.options.target_dir.string());
        
        int result = std::system(cmd.c_str());
        if (result != 0) {
            Logger::instance().error(std::format("Extraction failed with code {}", result));
            return false;
        }

        Logger::instance().info("Cleaning up temporary package...");
        std::filesystem::remove(zip_path);

        return true;
    } catch (const std::exception& e) {
        Logger::instance().error(std::format("Extraction exception: {}", e.what()));
        return false;
    }
}

}
