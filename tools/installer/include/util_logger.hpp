#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <chrono>
#include <format>

namespace druk::installer {

enum class LogLevel {
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void log(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::system_clock::now();
        std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);
        
        std::string level_str;
        switch (level) {
            case LogLevel::Info:    level_str = "INFO"; break;
            case LogLevel::Warning: level_str = "WARN"; break;
            case LogLevel::Error:   level_str = "ERR "; break;
        }

        std::string entry = std::format("[{}] [{}] {}\n", timestamp, level_str, message);
        
        recent_logs_.push_back(std::format("[{}] {}", level_str, message));

        if (file_.is_open()) {
            file_ << entry;
            file_.flush();
        }
        
        #ifdef _DEBUG
        OutputDebugStringA(entry.c_str());
        #endif
    }

    void info(const std::string& msg) { log(LogLevel::Info, msg); }
    void warn(const std::string& msg) { log(LogLevel::Warning, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }

    std::vector<std::string> get_recent_logs(size_t max_count) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (recent_logs_.size() <= max_count) return recent_logs_;
        return std::vector<std::string>(recent_logs_.end() - max_count, recent_logs_.end());
    }

private:
    Logger() {
        char temp_path[MAX_PATH];
        if (GetTempPathA(MAX_PATH, temp_path)) {
            log_path_ = std::filesystem::path(temp_path) / "druk_installer.log";
            file_.open(log_path_, std::ios::app);
            info("--- Installer Session Started ---");
        }
    }

    ~Logger() {
        if (file_.is_open()) {
            info("--- Installer Session Ended ---");
            file_.close();
        }
    }

    std::filesystem::path log_path_;
    std::ofstream file_;
    std::mutex mutex_;
    std::vector<std::string> recent_logs_;
};

}
