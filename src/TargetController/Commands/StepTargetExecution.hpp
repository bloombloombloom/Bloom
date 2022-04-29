#pragma once

#include <optional>

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class StepTargetExecution: public Command
    {
    public:
        static constexpr CommandType type = CommandType::STEP_TARGET_EXECUTION;
        static inline const std::string name = "StepTargetExecution";

        std::optional<std::uint32_t> fromProgramCounter;

        StepTargetExecution() = default;
        explicit StepTargetExecution(std::uint32_t fromProgramCounter)
            : fromProgramCounter(fromProgramCounter)
        {};

        [[nodiscard]] CommandType getType() const override {
            return StepTargetExecution::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
