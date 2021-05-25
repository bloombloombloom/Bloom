#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class ResumeTargetExecution: public Event
    {
    public:
        static inline const std::string name = "ResumeTargetExecutionEvent";
        std::optional<std::uint32_t> fromProgramCounter;

        std::string getName() const override {
            return ResumeTargetExecution::name;
        }

        ResumeTargetExecution() = default;
        ResumeTargetExecution(std::uint32_t fromProgramCounter): fromProgramCounter(fromProgramCounter) {};
    };
}
