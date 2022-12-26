#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <optional>
#include <cstdint>

#include "src/Services/DateTimeService.hpp"

namespace Bloom::Events
{
    static_assert(std::atomic<int>::is_always_lock_free);

    enum class EventType: std::uint8_t
    {
        GENERIC,
        DEBUG_SESSION_STARTED,
        DEBUG_SESSION_FINISHED,
        TARGET_CONTROLLER_THREAD_STATE_CHANGED,
        TARGET_CONTROLLER_STATE_CHANGED,
        SHUTDOWN_TARGET_CONTROLLER,
        TARGET_CONTROLLER_ERROR_OCCURRED,
        SHUTDOWN_APPLICATION,
        DEBUG_SERVER_THREAD_STATE_CHANGED,
        SHUTDOWN_DEBUG_SERVER,
        REGISTERS_WRITTEN_TO_TARGET,
        TARGET_EXECUTION_RESUMED,
        TARGET_EXECUTION_STOPPED,
        MEMORY_WRITTEN_TO_TARGET,
        INSIGHT_THREAD_STATE_CHANGED,
        TARGET_RESET,
        PROGRAMMING_MODE_ENABLED,
        PROGRAMMING_MODE_DISABLED,
    };

    class Event
    {
    public:
        int id = ++(Event::lastEventId);
        QDateTime createdTimestamp = Services::DateTimeService::currentDateTime();

        static constexpr EventType type = EventType::GENERIC;
        static const inline std::string name = "GenericEvent";

        Event() = default;
        virtual ~Event() = default;

        Event(const Event& other) = default;
        Event(Event&& other) = default;

        Event& operator = (const Event& other) = default;
        Event& operator = (Event&& other) = default;

        [[nodiscard]] virtual std::string getName() const {
            return Event::name;
        }

        [[nodiscard]] virtual EventType getType() const {
            return Event::type;
        }

    private:
        static inline std::atomic<int> lastEventId = 0;
    };
}
