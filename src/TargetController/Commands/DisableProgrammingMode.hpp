#pragma once

#include "Command.hpp"

namespace TargetController::Commands
{
    class DisableProgrammingMode: public Command
    {
    public:
        static constexpr CommandType type = CommandType::DISABLE_PROGRAMMING_MODE;
        static const inline std::string name = "DisableProgrammingMode";

        DisableProgrammingMode() = default;

        [[nodiscard]] CommandType getType() const override {
            return DisableProgrammingMode::type;
        }

        [[nodiscard]] bool requiresStoppedTargetState() const override {
            return true;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return false;
        }
    };
}
