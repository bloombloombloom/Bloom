#include "Logger.hpp"

#include <iostream>

void Logger::configure(const ProjectConfig& projectConfig) {
    if (projectConfig.debugLogging) {
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

void Logger::log(const std::string& message, LogLevel level) {
    using Services::DateTimeService;
    const auto timestamp = DateTimeService::currentDateTime();
    const auto threadName = Logger::threadName();
    static const auto timezoneAbbreviation = DateTimeService::getTimeZoneAbbreviation(timestamp).toStdString();

    const auto lock = std::unique_lock{Logger::printMutex};

    // Print the timestamp and id in a green font color:
    std::cout << "\033[32m";
    std::cout << timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz ").toStdString()
        + timezoneAbbreviation;

    if (!threadName.empty()) {
        std::cout << " [" << threadName << "]";
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

    std::cout << message << "\033[0m" << std::endl;
}

const std::string& Logger::threadName() {
    static auto nameCache = std::map<::pthread_t, std::string>{};

    const auto threadId = ::pthread_self();

    auto nameIt = nameCache.find(threadId);
    if (nameIt == nameCache.end()) {
        auto threadNameBuf = std::array<char, 16>{};

        if (::pthread_getname_np(::pthread_self(), threadNameBuf.data(), threadNameBuf.size()) != 0) {
            static const auto emptyName = std::string{};
            return emptyName;
        }

        const auto name = std::string{threadNameBuf.data()};

        /*
         * The name of the main thread is also the name of the process, so we have to name the
         * main thread "Bloom" (to prevent confusion).
         *
         * We override the main thread name when printing logs, to keep the format of the thread name in the
         * logs consistent.
         */
        nameIt = nameCache.emplace(threadId, name == "Bloom" ? "MT" : name).first;
    }

    return nameIt->second;
}
