#pragma once

#include <optional>

#include "Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::TargetController::Commands
{
    class StepTargetExecution: public Command
    {
    public:
        static constexpr CommandType type = CommandType::STEP_TARGET_EXECUTION;
        static const inline std::string name = "StepTargetExecution";

        std::optional<Targets::TargetProgramCounter> fromProgramCounter;

        StepTargetExecution() = default;
        explicit StepTargetExecution(Targets::TargetProgramCounter fromProgramCounter)
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
