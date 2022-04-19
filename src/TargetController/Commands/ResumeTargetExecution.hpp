#pragma once

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class ResumeTargetExecution: public Command
    {
    public:
        static constexpr CommandType type = CommandType::RESUME_TARGET_EXECUTION;
        static inline const std::string name = "ResumeTargetExecution";

        std::optional<std::uint32_t> fromProgramCounter;

        ResumeTargetExecution() = default;
        explicit ResumeTargetExecution(std::uint32_t fromProgramCounter)
            : fromProgramCounter(fromProgramCounter)
        {};

        [[nodiscard]] CommandType getType() const override {
            return ResumeTargetExecution::type;
        }
    };
}
