#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class StepTargetExecution: public Event
    {
    public:
        static inline EventType type = EventType::STEP_TARGET_EXECUTION;
        static inline const std::string name = "StepTargetExecution";
        std::optional<std::uint32_t> fromProgramCounter;

        StepTargetExecution() = default;
        explicit StepTargetExecution(std::uint32_t fromProgramCounter): fromProgramCounter(fromProgramCounter) {};

        [[nodiscard]] EventType getType() const override {
            return StepTargetExecution::type;
        }

        [[nodiscard]] std::string getName() const override {
            return StepTargetExecution::name;
        }
    };
}
