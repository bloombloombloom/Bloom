#pragma once

#include <cstdint>

#include "Event.hpp"
#include "src/Targets/Target.hpp"

namespace Bloom::Events
{
    using Targets::TargetBreakCause;

    class TargetExecutionStopped: public Event
    {
    public:
        static inline const std::string name = "TargetExecutionStopped";
        std::uint32_t programCounter;
        TargetBreakCause breakCause;

        TargetExecutionStopped(std::uint32_t programCounter, TargetBreakCause breakCause) :
        programCounter(programCounter), breakCause(breakCause) {}

        std::string getName() const override {
            return TargetExecutionStopped::name;
        }
    };
}
