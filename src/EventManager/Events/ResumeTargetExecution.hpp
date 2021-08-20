#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "TargetExecutionResumed.hpp"

namespace Bloom::Events
{
    class ResumeTargetExecution: public Event
    {
    public:
        using TargetControllerResponseType = TargetExecutionResumed;

        static inline EventType type = EventType::RESUME_TARGET_EXECUTION;
        static inline const std::string name = "ResumeTargetExecutionEvent";
        std::optional<std::uint32_t> fromProgramCounter;

        ResumeTargetExecution() = default;
        explicit ResumeTargetExecution(std::uint32_t fromProgramCounter): fromProgramCounter(fromProgramCounter) {};

        [[nodiscard]] EventType getType() const override {
            return ResumeTargetExecution::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ResumeTargetExecution::name;
        }
    };
}
