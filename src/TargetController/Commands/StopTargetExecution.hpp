#pragma once

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class StopTargetExecution: public Command
    {
    public:
        static constexpr CommandType type = CommandType::STOP_TARGET_EXECUTION;
        static inline const std::string name = "StopTargetExecution";

        [[nodiscard]] CommandType getType() const override {
            return StopTargetExecution::type;
        }
    };
}
