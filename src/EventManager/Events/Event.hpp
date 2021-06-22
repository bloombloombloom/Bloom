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
        static inline std::atomic<int> lastEventId = 0;

    public:
        int id = ++(Event::lastEventId);
        QDateTime createdTimestamp = DateTime::currentDateTime();
        std::optional<int> correlationId;

        static inline const std::string name = "GenericEvent";

        [[nodiscard]] virtual std::string getName() const {
            return Event::name;
        }
    };
}
