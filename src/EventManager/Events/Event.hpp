#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <optional>
#include <cstdint>

#include "src/Helpers/DateTime.hpp"

namespace Bloom::Events
{
    static_assert(std::atomic<int>::is_always_lock_free);

    enum class EventType: std::uint8_t
    {
        GENERIC,
        RESET_TARGET,
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
        SET_BREAKPOINT_ON_TARGET,
        REMOVE_BREAKPOINT_ON_TARGET,
        BREAKPOINT_SET_ON_TARGET,
        BREAKPOINT_REMOVED_ON_TARGET,
        SET_PROGRAM_COUNTER_ON_TARGET,
        PROGRAM_COUNTER_SET_ON_TARGET,
        EXTRACT_TARGET_DESCRIPTOR,
        TARGET_DESCRIPTOR_EXTRACTED,
        INSIGHT_THREAD_STATE_CHANGED,
        RETRIEVE_TARGET_PIN_STATES,
        TARGET_PIN_STATES_RETRIEVED,
        SET_TARGET_PIN_STATE,
        RETRIEVE_STACK_POINTER_FROM_TARGET,
        STACK_POINTER_RETRIEVED_FROM_TARGET,
        TARGET_RESET,
    };

    class Event
    {
    public:
        int id = ++(Event::lastEventId);
        QDateTime createdTimestamp = DateTime::currentDateTime();
        std::optional<int> correlationId;

        static constexpr EventType type = EventType::GENERIC;
        static inline const std::string name = "GenericEvent";

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
