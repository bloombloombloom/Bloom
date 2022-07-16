#pragma once

#include <mutex>
#include <QDateTime>

namespace Bloom
{
    /**
     * Some (maybe all) QDateTime static functions are not thread-safe and thus can result in data races.
     * This trivial helper class wraps some of these functions and employs a mutex to prevent data races.
     */
    class DateTime
    {
    public:
        /**
         * The QDateTime::currentDateTime() static function is not thread-safe. This may be caused by the
         * underlying interfacing with the system clock.
         *
         * @return
         */
        static QDateTime currentDateTime() {
            const auto lock = std::unique_lock(DateTime::systemClockMutex);
            return QDateTime::currentDateTime();
        }

        /**
         * The QDateTime::timeZoneAbbreviation() is a non-static member function but it may still interface with the
         * system clock. This can result in race conditions when called simultaneously to QDateTime::currentDateTime(),
         * and so any calls to it must require possession of the mutex.
         *
         * @param dateTime
         * @return
         */
        static QString getTimeZoneAbbreviation(const QDateTime& dateTime) {
            const auto lock = std::unique_lock(DateTime::systemClockMutex);
            return dateTime.timeZoneAbbreviation();
        }

    private:
        static inline std::mutex systemClockMutex;
    };
}
