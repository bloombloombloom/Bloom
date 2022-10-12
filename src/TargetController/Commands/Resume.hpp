#pragma once

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class Resume: public Command
    {
    public:
        static constexpr CommandType type = CommandType::RESUME;
        static const inline std::string name = "Resume";

        [[nodiscard]] CommandType getType() const override {
            return Resume::type;
        }

        [[nodiscard]] bool requiresDebugMode() const override {
            return false;
        }
    };
}
