#pragma once

#include <cstdint>
#include <string>
#include <mutex>
#include <atomic>

#include "src/ProjectConfig.hpp"
#include "src/Services/DateTimeService.hpp"

namespace Bloom
{
    static_assert(std::atomic<std::uint32_t>::is_always_lock_free);

    enum class LogLevel: std::uint8_t
    {
        INFO,
        WARNING,
        ERROR,
        DEBUG,
    };

    class LogEntry
    {
    public:
        std::uint32_t id = ++(LogEntry::lastLogId);
        std::string message;
        LogLevel logLevel;
        QDateTime timestamp = Services::DateTimeService::currentDateTime();
        std::string threadName;

        LogEntry(std::string message, LogLevel logLevel)
            : message(std::move(message))
            , logLevel(logLevel)
        {
            // Get thread name
            std::array<char, 16> threadNameBuf = {};

            if (::pthread_getname_np(::pthread_self(), threadNameBuf.data(), threadNameBuf.size()) == 0) {
                /*
                 * The name of the main thread is also the name of the process, so we have to name the
                 * main thread "Bloom" (to prevent confusion).
                 *
                 * We override the main thread name when printing logs, to keep the format of the thread name in the
                 * logs consistent.
                 */
                this->threadName = std::string(threadNameBuf.data());
                this->threadName = this->threadName == "Bloom" ? "MT" : this->threadName;
            }
        };

    private:
        static inline std::atomic<std::uint32_t> lastLogId = 0;
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
                Logger::log(LogEntry(message, LogLevel::INFO));
            }
        }

        static void warning(const std::string& message) {
            if (Logger::warningPrintingEnabled) {
                Logger::log(LogEntry(message, LogLevel::WARNING));
            }
        }

        static void error(const std::string& message) {
            if (Logger::errorPrintingEnabled) {
                Logger::log(LogEntry(message, LogLevel::ERROR));
            }
        }

        static void debug(const std::string& message) {
            if (Logger::debugPrintingEnabled) {
                Logger::log(LogEntry(message, LogLevel::DEBUG));
            }
        }

    private:
        static inline bool errorPrintingEnabled = true;
        static inline bool warningPrintingEnabled = true;
        static inline bool infoPrintingEnabled = true;
        static inline bool debugPrintingEnabled = false;

        static inline std::mutex printMutex;

        static void log(const LogEntry& logEntry);
    };
}
