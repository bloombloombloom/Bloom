#pragma once

#include "Command.hpp"

namespace Bloom::TargetController::Commands
{
    class ResetTarget: public Command
    {
    public:
        static constexpr CommandType type = CommandType::RESET_TARGET;
        static inline const std::string name = "ResetTarget";

        [[nodiscard]] CommandType getType() const override {
            return ResetTarget::type;
        }
    };
}
