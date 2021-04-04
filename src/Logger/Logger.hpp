#pragma once

#include <memory>
#include <map>
#include <string>
#include <functional>
#include <QTimeZone>
#include <mutex>

#include "src/ApplicationConfig.hpp"
#include "src/Helpers/DateTime.hpp"

namespace Bloom
{
    enum class LogLevel
    {
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        DEBUG = 4,
    };

    struct LogEntry {
        std::string threadName;
        std::string message;
        LogLevel logLevel;
        QDateTime timestamp = DateTime::currentDateTime();

        LogEntry(std::string message, LogLevel logLevel): message(std::move(message)), logLevel(logLevel) {
            // Get thread name
            std::array<char, 16> threadNameBuf;

            if (pthread_getname_np(pthread_self(), threadNameBuf.data(), threadNameBuf.size()) == 0) {
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
    };

    /**
     * Super simple thread safe static Logger class for basic logging.
     *
     * TODO: Add the ability to dump the logs to a file.
     */
    class Logger
    {
    private:
        /**
         * We keep a record of every log entry for future processing. Maybe dumping to a file or something
         * of that nature when a fatal error occurs.
         */
        static inline std::vector<LogEntry> logEntries;

        static inline bool errorPrintingEnabled = true;
        static inline bool warningPrintingEnabled = true;
        static inline bool infoPrintingEnabled = true;
        static inline bool debugPrintingEnabled = false;

        static inline std::mutex logMutex;

        static void log(const std::string& message, LogLevel logLevel, bool print);

    public:
        static void setInfoPrinting(bool enabled) {
            Logger::infoPrintingEnabled = enabled;
        }

        static void setWarningPrinting(bool enabled) {
            Logger::warningPrintingEnabled = enabled;
        }

        static void setErrorPrinting(bool enabled) {
            Logger::errorPrintingEnabled = enabled;
        }

        static void info(const std::string& message) {
            Logger::log(message, LogLevel::INFO, Logger::infoPrintingEnabled);
        }

        static void warning(const std::string& message) {
            Logger::log(message, LogLevel::WARNING, Logger::warningPrintingEnabled);
        }

        static void error(const std::string& message) {
            Logger::log(message, LogLevel::ERROR, Logger::errorPrintingEnabled);
        }

        static void debug(const std::string& message) {
            Logger::log(message, LogLevel::DEBUG, Logger::debugPrintingEnabled);
        }

        static void configure(ApplicationConfig& applicationConfig);

        static void silence();
    };
}
