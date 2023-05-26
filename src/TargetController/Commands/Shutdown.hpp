#pragma once

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class Shutdown: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SHUTDOWN;
        static const inline std::string name = "Shutdown";

        [[nodiscard]] CommandType getType() const override {
            return Shutdown::type;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return false;
        }
    };
}
