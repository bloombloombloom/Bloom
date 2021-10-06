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

    enum EventType: std::uint8_t
    {
        GENERIC,
        STOP_TARGET_EXECUTION,
        RESUME_TARGET_EXECUTION,
        RESET_TARGET,
        DEBUG_SESSION_STARTED,
        DEBUG_SESSION_FINISHED,
        TARGET_CONTROLLER_THREAD_STATE_CHANGED,
        REPORT_TARGET_CONTROLLER_STATE,
        TARGET_CONTROLLER_STATE_REPORTED,
        SHUTDOWN_TARGET_CONTROLLER,
        TARGET_CONTROLLER_ERROR_OCCURRED,
        SHUTDOWN_APPLICATION,
        DEBUG_SERVER_THREAD_STATE_CHANGED,
        SHUTDOWN_DEBUG_SERVER,
        RETRIEVE_REGISTERS_FROM_TARGET,
        REGISTERS_RETRIEVED_FROM_TARGET,
        WRITE_REGISTERS_TO_TARGET,
        REGISTERS_WRITTEN_TO_TARGET,
        TARGET_EXECUTION_RESUMED,
        TARGET_EXECUTION_STOPPED,
        RETRIEVE_MEMORY_FROM_TARGET,
        MEMORY_RETRIEVED_FROM_TARGET,
        WRITE_MEMORY_TO_TARGET,
        MEMORY_WRITTEN_TO_TARGET,
        SET_BREAKPOINT_ON_TARGET,
        REMOVE_BREAKPOINT_ON_TARGET,
        BREAKPOINT_SET_ON_TARGET,
        BREAKPOINT_REMOVED_ON_TARGET,
        STEP_TARGET_EXECUTION,
        SET_PROGRAM_COUNTER_ON_TARGET,
        PROGRAM_COUNTER_SET_ON_TARGET,
        EXTRACT_TARGET_DESCRIPTOR,
        TARGET_DESCRIPTOR_EXTRACTED,
        INSIGHT_THREAD_STATE_CHANGED,
        RETRIEVE_TARGET_PIN_STATES,
        TARGET_PIN_STATES_RETRIEVED,
        SET_TARGET_PIN_STATE,
    };

    class Event
    {
    public:
        int id = ++(Event::lastEventId);
        QDateTime createdTimestamp = DateTime::currentDateTime();
        std::optional<int> correlationId;

        static inline EventType type = EventType::GENERIC;
        static inline const std::string name = "GenericEvent";

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
