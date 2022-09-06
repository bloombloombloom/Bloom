#pragma once

#include <optional>

#include "Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::TargetController::Commands
{
    class ResumeTargetExecution: public Command
    {
    public:
        static constexpr CommandType type = CommandType::RESUME_TARGET_EXECUTION;
        static inline const std::string name = "ResumeTargetExecution";

        std::optional<Targets::TargetProgramCounter> fromProgramCounter;

        ResumeTargetExecution() = default;
        explicit ResumeTargetExecution(Targets::TargetProgramCounter fromProgramCounter)
            : fromProgramCounter(fromProgramCounter)
        {};

        [[nodiscard]] CommandType getType() const override {
            return ResumeTargetExecution::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
