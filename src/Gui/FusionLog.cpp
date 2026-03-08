// SPDX-License-Identifier: LGPL-2.1-or-later
// UniCAD Logging System - Implementation

#include "FusionLog.h"
#include <App/Application.h>
#include <filesystem>

namespace Gui {

void FusionLog::setFileLogging(bool enable, const std::string& path)
{
    std::lock_guard<std::mutex> lock(fileMutex);
    fileEnabled = enable;

    if (fileStream.is_open()) {
        fileStream.close();
    }

    if (enable) {
        std::string logPath = path;
        if (logPath.empty()) {
            // Default: write to user data dir
            logPath = App::Application::getUserAppDataDir() + "/UniCAD.log";
        }

        // Ensure directory exists
        auto dir = std::filesystem::path(logPath).parent_path();
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directories(dir);
        }

        fileStream.open(logPath, std::ios::out | std::ios::app);
        if (fileStream.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            char timeBuf[26];
            struct tm tmBuf;
#ifdef _WIN32
            localtime_s(&tmBuf, &time);
#else
            localtime_r(&time, &tmBuf);
#endif
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &tmBuf);
            fileStream << "\n=== UniCAD Log Session Started " << timeBuf << " ===\n";
            fileStream.flush();
        }
    }
}

} // namespace Gui
