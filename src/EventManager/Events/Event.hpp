#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <optional>

#include "src/Helpers/DateTime.hpp"

namespace Bloom::Events
{
    static_assert(std::atomic<int>::is_always_lock_free);

    class Event
    {
    private:
        QDateTime createdTimestamp = DateTime::currentDateTime();
        static inline std::atomic<int> lastEventId = 0;

    public:
        int id = ++(this->lastEventId);
        std::optional<int> correlationId;

        static inline const std::string name = "GenericEvent";

        virtual std::string getName() const {
            return Event::name;
        }

        long getCreatedEpochTimestamp() const {
            return this->createdTimestamp.toMSecsSinceEpoch();
        }
    };
}
