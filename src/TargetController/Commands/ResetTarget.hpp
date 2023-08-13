#pragma once

#include "Command.hpp"

namespace TargetController::Commands
{
    class ResetTarget: public Command
    {
    public:
        static constexpr CommandType type = CommandType::RESET_TARGET;
        static const inline std::string name = "ResetTarget";

        [[nodiscard]] CommandType getType() const override {
            return ResetTarget::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }
    };
}
