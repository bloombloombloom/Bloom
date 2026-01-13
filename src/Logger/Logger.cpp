#include "Logger.hpp"

#include <iostream>

void Logger::configure(const ProjectConfig& projectConfig) {
    if (projectConfig.debugLogging) {
        Logger::debugPrintingEnabled = true;
    }
}

void Logger::silence() {
    Logger::debugPrintingEnabled = false;
    Logger::infoPrintingEnabled = false;
    Logger::errorPrintingEnabled = false;
    Logger::warningPrintingEnabled = false;
}

void Logger::log(const std::string& message, LogLevel level, std::optional<std::source_location> sourceLocation) {
    using Services::DateTimeService;
    const auto timestamp = DateTimeService::currentDateTime();
    static const auto timezoneAbbreviation = DateTimeService::getTimeZoneAbbreviation(timestamp).toStdString();

    const auto lock = std::unique_lock{Logger::printMutex};

    // Print the timestamp and id in a green font color:
    std::cout << "\033[32m";
    std::cout << timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz ").toStdString()
        + timezoneAbbreviation;

    if (!Logger::threadName.empty()) {
        std::cout << " [" << Logger::threadName << "]";
    }

    std::cout << ": \033[0m";

    switch (level) {
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

    if (sourceLocation.has_value()) {
        std::cout << "[" << sourceLocation->file_name() << ":" << sourceLocation->line() << "] ";
    }

    std::cout << message << "\033[0m" << std::endl;
}
