#include <iostream>
#include <thread>
#include "Logger.hpp"

using namespace Bloom;

void Logger::log(const std::string& message, LogLevel logLevel, bool print) {
    auto lock = std::unique_lock(Logger::logMutex);
    auto logEntry = LogEntry(message, logLevel);
    Logger::logEntries.push_back(logEntry);
    auto index = Logger::logEntries.size();

    if (print) {
        // Print the timestamp and index in a green font color:
        std::cout << "\033[32m";
        std::cout << logEntry.timestamp.toString("yyyy-MM-dd hh:mm:ss ").toStdString() +
            DateTime::getTimeZoneAbbreviation(logEntry.timestamp).toStdString();

        if (!logEntry.threadName.empty()) {
            std::cout << " [" << logEntry.threadName << "]";
        }

        // The index serves as an ID for each log entry. Just here for convenience when referencing specific log entries.
        std::cout << " [" << index << "]: ";
        std::cout << "\033[0m";

        switch (logLevel) {
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
}

void Logger::configure(ApplicationConfig& applicationConfig) {
    if (applicationConfig.debugLoggingEnabled) {
        Logger::debugPrintingEnabled = true;
        Logger::debug("Debug log printing has been enabled.");
    }
}

void Logger::silence() {
    Logger::debugPrintingEnabled = false;
    Logger::infoPrintingEnabled = false;
    Logger::errorPrintingEnabled = false;
    Logger::warningPrintingEnabled = false;
}
