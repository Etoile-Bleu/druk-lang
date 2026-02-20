#include "logic_engine.hpp"
#include <windows.h>

namespace druk::installer {

bool InstallationEngine::update_system_path() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &key) == ERROR_SUCCESS) {
        char current_path[4096] = {0};
        DWORD size = sizeof(current_path);
        DWORD type = 0;
        
        RegQueryValueExA(key, "Path", nullptr, &type, (LPBYTE)current_path, &size);
        
        std::string new_path = current_path;
        std::string bin_dir = (context_.options.target_dir / "bin").string();
        
        if (new_path.find(bin_dir) == std::string::npos) {
            if (!new_path.empty() && new_path.back() != ';') new_path += ";";
            new_path += bin_dir;
            
            if (RegSetValueExA(key, "Path", 0, REG_EXPAND_SZ, (const BYTE*)new_path.c_str(), (DWORD)new_path.length() + 1) == ERROR_SUCCESS) {
                SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, nullptr);
                RegCloseKey(key);
                return true;
            }
        }
        RegCloseKey(key);
    }
    return false; 
}

bool InstallationEngine::remove_system_path() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &key) == ERROR_SUCCESS) {
        char current_path[4096] = {0};
        DWORD size = sizeof(current_path);
        DWORD type = 0;
        
        RegQueryValueExA(key, "Path", nullptr, &type, (LPBYTE)current_path, &size);
        
        std::string path_str = current_path;
        std::string bin_dir = (context_.options.target_dir / "bin").string();
        
        size_t pos = path_str.find(bin_dir);
        if (pos != std::string::npos) {
            if (pos > 0 && path_str[pos-1] == ';') {
                path_str.erase(pos - 1, bin_dir.length() + 1);
            } else if (pos + bin_dir.length() < path_str.length() && path_str[pos + bin_dir.length()] == ';') {
                path_str.erase(pos, bin_dir.length() + 1);
            } else {
                path_str.erase(pos, bin_dir.length());
            }

            if (RegSetValueExA(key, "Path", 0, REG_EXPAND_SZ, (const BYTE*)path_str.c_str(), (DWORD)path_str.length() + 1) == ERROR_SUCCESS) {
                SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Environment", SMTO_ABORTIFHUNG, 5000, nullptr);
            }
        }
        RegCloseKey(key);
        return true;
    }
    return false;
}

bool InstallationEngine::register_uninstaller() {
    std::string key_path = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\DrukCompiler";
    HKEY key;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, key_path.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr) == ERROR_SUCCESS) {
        std::string display_name = "Druk Compiler 1.0.0";
        std::string uninstall_string = (context_.options.target_dir / "bin" / "druk-installer.exe").string();
        std::string install_location = context_.options.target_dir.string();

        RegSetValueExA(key, "DisplayName", 0, REG_SZ, (const BYTE*)display_name.c_str(), (DWORD)display_name.length() + 1);
        RegSetValueExA(key, "UninstallString", 0, REG_SZ, (const BYTE*)uninstall_string.c_str(), (DWORD)uninstall_string.length() + 1);
        RegSetValueExA(key, "InstallLocation", 0, REG_SZ, (const BYTE*)install_location.c_str(), (DWORD)install_location.length() + 1);
        RegCloseKey(key);
        return true;
    }
    return false;
}

bool InstallationEngine::unregister_uninstaller() {
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\DrukCompiler");
    return true;
}

bool InstallationEngine::create_shortcuts() { return true; }

bool InstallationEngine::associate_file_extensions() {
    HKEY key;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Classes\\.druk", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr) == ERROR_SUCCESS) {
        RegSetValueExA(key, nullptr, 0, REG_SZ, (const BYTE*)"Druk.SourceFile", 16);
        RegCloseKey(key);
        if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Classes\\Druk.SourceFile\\shell\\open\\command", 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &key, nullptr) == ERROR_SUCCESS) {
            std::string cmd = "\"" + (context_.options.target_dir / "bin" / "druk.exe").string() + "\" \"%1\"";
            RegSetValueExA(key, nullptr, 0, REG_SZ, (const BYTE*)cmd.c_str(), (DWORD)cmd.length() + 1);
            RegCloseKey(key);
            return true;
        }
    }
    return false;
}

bool InstallationEngine::remove_file_associations() {
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\.druk");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\Druk.SourceFile\\shell\\open\\command");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\Druk.SourceFile\\shell\\open");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\Druk.SourceFile\\shell");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\Druk.SourceFile");
    return true;
}

}
