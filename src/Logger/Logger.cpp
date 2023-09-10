#include "Logger.hpp"

#include <iostream>

void Logger::configure(const ProjectConfig& projectConfig) {
    if (projectConfig.debugLoggingEnabled) {
        Logger::debugPrintingEnabled = true;
        Logger::debug("Debug log printing has been enabled");
    }
}

void Logger::silence() {
    Logger::debugPrintingEnabled = false;
    Logger::infoPrintingEnabled = false;
    Logger::errorPrintingEnabled = false;
    Logger::warningPrintingEnabled = false;
}

void Logger::log(const LogEntry& logEntry) {
    static auto timezoneAbbreviation = Services::DateTimeService::getTimeZoneAbbreviation(
        logEntry.timestamp
    ).toStdString();

    const auto lock = std::unique_lock(Logger::printMutex);

    // Print the timestamp and id in a green font color:
    std::cout << "\033[32m";
    std::cout << logEntry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz ").toStdString()
        + timezoneAbbreviation;

    if (!logEntry.threadName.empty()) {
        std::cout << " [" << logEntry.threadName << "]";
    }

    std::cout << " [" << logEntry.id << "]: ";
    std::cout << "\033[0m";

    switch (logEntry.logLevel) {
        case LogLevel::ERROR: {
            // Errors in red
            std::cout << "\033[31m";
            std::cout << "[ERROR] ";
            break;
        }
        case LogLevel::WARNING: {
            // Warnings in yellow
            std::cout << "\033[33m";
            std::cout << "[WARNING] ";
            break;
        }
        case LogLevel::INFO: {
            std::cout << "[INFO] ";
            break;
        }
        case LogLevel::DEBUG: {
            std::cout << "[DEBUG] ";
            break;
        }
    }

    std::cout << logEntry.message << "\033[0m" << std::endl;
}
