#pragma once

#include "Command.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace TargetController::Commands
{
    class ResumeTargetExecution: public Command
    {
    public:
        static constexpr CommandType type = CommandType::RESUME_TARGET_EXECUTION;
        static const inline std::string name = "ResumeTargetExecution";

        ResumeTargetExecution() = default;

        [[nodiscard]] CommandType getType() const override {
            return ResumeTargetExecution::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
