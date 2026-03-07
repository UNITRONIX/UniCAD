// SPDX-License-Identifier: LGPL-2.1-or-later
// FusionCAD Logging System
// Provides structured logging for debugging FusionCAD-specific features

#ifndef GUI_FUSIONLOG_H
#define GUI_FUSIONLOG_H

#include <Base/Console.h>
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <FCGlobal.h>

namespace Gui {

/// FusionCAD structured logging to console and optional file
class GuiExport FusionLog
{
public:
    enum class Level { Debug, Info, Warning, Error };

    static FusionLog& instance() {
        static FusionLog inst;
        return inst;
    }

    /// Enable/disable file logging
    void setFileLogging(bool enable, const std::string& path = "");

    /// Set minimum log level
    void setLevel(Level level) { minLevel = level; }

    /// Log a message with category tag
    template<typename... Args>
    void log(Level level, const char* category, const char* fmt, Args&&... args) {
        if (level < minLevel) return;

        std::string prefix = formatPrefix(level, category);
        std::string msg = prefix + fmt;

        switch (level) {
            case Level::Debug:
                Base::Console().log(msg.c_str(), std::forward<Args>(args)...);
                break;
            case Level::Info:
                Base::Console().message(msg.c_str(), std::forward<Args>(args)...);
                break;
            case Level::Warning:
                Base::Console().warning(msg.c_str(), std::forward<Args>(args)...);
                break;
            case Level::Error:
                Base::Console().error(msg.c_str(), std::forward<Args>(args)...);
                break;
        }

        if (fileEnabled && fileStream.is_open()) {
            writeToFile(prefix, fmt, std::forward<Args>(args)...);
        }
    }

    // Convenience methods
    template<typename... Args>
    void debug(const char* category, const char* fmt, Args&&... args) {
        log(Level::Debug, category, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void info(const char* category, const char* fmt, Args&&... args) {
        log(Level::Info, category, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void warn(const char* category, const char* fmt, Args&&... args) {
        log(Level::Warning, category, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void error(const char* category, const char* fmt, Args&&... args) {
        log(Level::Error, category, fmt, std::forward<Args>(args)...);
    }

private:
    FusionLog() = default;
    ~FusionLog() { if (fileStream.is_open()) fileStream.close(); }
    FusionLog(const FusionLog&) = delete;
    FusionLog& operator=(const FusionLog&) = delete;

    std::string formatPrefix(Level level, const char* category) {
        const char* levelStr = "";
        switch (level) {
            case Level::Debug:   levelStr = "DBG"; break;
            case Level::Info:    levelStr = "INF"; break;
            case Level::Warning: levelStr = "WRN"; break;
            case Level::Error:   levelStr = "ERR"; break;
        }
        return std::string("[FusionCAD/") + category + "/" + levelStr + "] ";
    }

    template<typename... Args>
    void writeToFile(const std::string& prefix, const char* fmt, Args&&... args) {
        std::lock_guard<std::mutex> lock(fileMutex);
        try {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            char timeBuf[26];
            struct tm tmBuf;
#ifdef _WIN32
            localtime_s(&tmBuf, &time);
#else
            localtime_r(&time, &tmBuf);
#endif
            strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tmBuf);
            fileStream << timeBuf << " " << prefix;
            // Write fmt directly (simplified - doesn't format args to file)
            fileStream << fmt << "\n";
            fileStream.flush();
        } catch (...) {}
    }

    Level minLevel = Level::Debug;
    bool fileEnabled = false;
    std::ofstream fileStream;
    std::mutex fileMutex;
};

// Macro shortcuts
#define FLOG_DEBUG(cat, ...) Gui::FusionLog::instance().debug(cat, __VA_ARGS__)
#define FLOG_INFO(cat, ...)  Gui::FusionLog::instance().info(cat, __VA_ARGS__)
#define FLOG_WARN(cat, ...)  Gui::FusionLog::instance().warn(cat, __VA_ARGS__)
#define FLOG_ERROR(cat, ...) Gui::FusionLog::instance().error(cat, __VA_ARGS__)

} // namespace Gui

#endif // GUI_FUSIONLOG_H
