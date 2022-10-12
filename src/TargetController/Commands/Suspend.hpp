#pragma once

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class Suspend: public Command
    {
    public:
        static constexpr CommandType type = CommandType::SUSPEND;
        static const inline std::string name = "Suspend";

        [[nodiscard]] CommandType getType() const override {
            return Suspend::type;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return false;
        }
    };
}
