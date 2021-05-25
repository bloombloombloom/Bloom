#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class StepTargetExecution: public Event
    {
    public:
        static inline const std::string name = "StepTargetExecution";
        std::optional<std::uint32_t> fromProgramCounter;

        std::string getName() const override {
            return StepTargetExecution::name;
        }

        StepTargetExecution() = default;
        StepTargetExecution(std::uint32_t fromProgramCounter): fromProgramCounter(fromProgramCounter) {};
    };
}
