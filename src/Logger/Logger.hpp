#pragma once

#include <cstdint>
#include <string>
#include <mutex>

#include "src/ProjectConfig.hpp"
#include "src/Services/DateTimeService.hpp"

enum class LogLevel: std::uint8_t
{
    INFO,
    WARNING,
    ERROR,
    DEBUG,
};

/**
 * Super simple thread safe static Logger class for basic logging.
 */
class Logger
{
public:
    static void configure(const ProjectConfig& projectConfig);

    static void silence();

    static void info(const std::string& message) {
        if (Logger::infoPrintingEnabled) {
            Logger::log(message, LogLevel::INFO);
        }
    }

    static void warning(const std::string& message) {
        if (Logger::warningPrintingEnabled) {
            Logger::log(message, LogLevel::WARNING);
        }
    }

    static void error(const std::string& message) {
        if (Logger::errorPrintingEnabled) {
            Logger::log(message, LogLevel::ERROR);
        }
    }

    static void debug(const std::string& message) {
        if (Logger::debugPrintingEnabled) {
            Logger::log(message, LogLevel::DEBUG);
        }
    }

private:
    static inline bool errorPrintingEnabled = true;
    static inline bool warningPrintingEnabled = true;
    static inline bool infoPrintingEnabled = true;
    static inline bool debugPrintingEnabled = false;

    static inline std::mutex printMutex;

    static void log(const std::string& message, LogLevel level);
    static const std::string& threadName();
};
