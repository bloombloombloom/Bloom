#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"

#include "src/Targets/TargetBreakpoint.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Events
{
    class TargetExecutionStopped: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_EXECUTION_STOPPED;
        static const inline std::string name = "TargetExecutionStopped";

        Targets::TargetMemoryAddress programCounter;
        Targets::TargetBreakCause breakCause;

        TargetExecutionStopped(Targets::TargetMemoryAddress programCounter, Targets::TargetBreakCause breakCause)
            : programCounter(programCounter)
            , breakCause(breakCause)
        {}

        [[nodiscard]] EventType getType() const override {
            return TargetExecutionStopped::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetExecutionStopped::name;
        }
    };
}
