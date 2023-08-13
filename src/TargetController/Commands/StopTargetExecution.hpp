#pragma once

#include "Command.hpp"

namespace TargetController::Commands
{
    class StopTargetExecution: public Command
    {
    public:
        static constexpr CommandType type = CommandType::STOP_TARGET_EXECUTION;
        static const inline std::string name = "StopTargetExecution";

        [[nodiscard]] CommandType getType() const override {
            return StopTargetExecution::type;
        }
    };
}
