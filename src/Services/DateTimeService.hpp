#pragma once

#include <mutex>
#include <QDateTime>
#include <QDate>

namespace Bloom::Services
{
    /**
     * Some (maybe all) QDateTime static functions are not thread-safe and thus can result in data races.
     * This trivial service class wraps some of these functions and employs a mutex to prevent data races.
     */
    class DateTimeService
    {
    public:
        /**
         * The QDateTime::currentDateTime() static function is not thread-safe. This may be caused by the
         * underlying interfacing with the system clock.
         *
         * @return
         */
        static QDateTime currentDateTime() {
            const auto lock = std::unique_lock(DateTimeService::systemClockMutex);
            return QDateTime::currentDateTime();
        }

        /**
         * This function calls QDateTime::currentDateTime(). See comment for DateTime::currentDateTime().
         *
         * @return
         */
        static QDate currentDate() {
            const auto lock = std::unique_lock(DateTimeService::systemClockMutex);
            return QDateTime::currentDateTime().date();
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
            const auto lock = std::unique_lock(DateTimeService::systemClockMutex);
            return dateTime.timeZoneAbbreviation();
        }

    private:
        static inline std::mutex systemClockMutex;
    };
}
