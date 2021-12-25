#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "src/Targets/Target.hpp"

namespace Bloom::Events
{
    class TargetExecutionStopped: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_EXECUTION_STOPPED;
        static inline const std::string name = "TargetExecutionStopped";
        std::uint32_t programCounter;
        Targets::TargetBreakCause breakCause;

        TargetExecutionStopped(std::uint32_t programCounter, Targets::TargetBreakCause breakCause) :
        programCounter(programCounter), breakCause(breakCause) {}

        [[nodiscard]] EventType getType() const override {
            return TargetExecutionStopped::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetExecutionStopped::name;
        }
    };
}
